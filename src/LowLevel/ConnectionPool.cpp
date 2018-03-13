#include <algorithm>
#include <core/sleep.hh>
#include <CQLDriver/Common/Exceptions/NotImplementedException.hpp>
#include <CQLDriver/Common/Exceptions/ConnectionNotAvailableException.hpp>
#include <CQLDriver/Common/Exceptions/LogicException.hpp>
#include <CQLDriver/Common/NodeCollection.hpp>
#include <CQLDriver/Common/SessionConfiguration.hpp>
#include "./ConnectionPool.hpp"
#include "./Connection.hpp"

namespace cql {
	/** Try to get a connection with idle stream, may return (nullptr, {}) if not available */
	std::pair<seastar::lw_shared_ptr<Connection>, ConnectionStream>
		ConnectionPool::tryGetConnection() {
		// find connection which has most free streams
		auto it = std::max_element(allConnections_.begin(), allConnections_.end(), [](auto& a, auto& b) {
			return a->getFreeStreamsCount() < b->getFreeStreamsCount();
		});
		if (it == allConnections_.end()) {
			return { nullptr, ConnectionStream() };
		}
		// get stream from the connection
		auto stream = (*it)->openStream();
		if (!stream.isValid()) {
			return { nullptr, ConnectionStream() };
		}
		return { *it, std::move(stream) };
	}

	/** Get a connection with idle stream, wait until they are available */
	seastar::future<seastar::lw_shared_ptr<Connection>, ConnectionStream>
		ConnectionPool::getConnection() {
		// always create new connection until min pool size is reached
		// call this function concurrently may creates more connections than min pool size,
		// but it's acceptable for now
		if (allConnections_.size() < sessionConfiguration_->getMinPoolSize()) {
			return makeConnection();
		}
		// use existing connections
		auto result = tryGetConnection();
		if (result.first.get() != nullptr) {
			return seastar::make_ready_future<
				seastar::lw_shared_ptr<Connection>, ConnectionStream>(
				result.first, std::move(result.second));
		}
		// create new connection if all existing connections is occupied until max pool size is reached
		if (allConnections_.size() < sessionConfiguration_->getMaxPoolSize()) {
			return makeConnection();
		}
		// wait until some connection is available
		seastar::promise<seastar::lw_shared_ptr<Connection>, ConnectionStream> promise;
		auto future = promise.get_future();
		if (!waiters_.push(std::move(promise))) {
			return seastar::make_exception_future<
				seastar::lw_shared_ptr<Connection>, ConnectionStream>(
				ConnectionNotAvailableException(CQL_CODEINFO, "no connections available"));
		} else {
			findIdleConnectionTimer();
		}
		return future;
	}

	/** Tell connection pool some connection become idle, this is optional */
	void ConnectionPool::notifyConnectionBecomeIdle(
		seastar::lw_shared_ptr<Connection>&& connection) {
		if (!waiters_.empty()) {
			auto promise = waiters_.pop();
			auto stream = connection->openStream();
			if (!stream.isValid()) {
				promise.set_exception(LogicException(CQL_CODEINFO,
					"open stream form a idle connection notified failed"));
			} else {
				promise.set_value(std::move(connection), std::move(stream));
			}
		}
	}

	/** Constructor */
	ConnectionPool::ConnectionPool(
		const seastar::lw_shared_ptr<SessionConfiguration>& sessionConfiguration,
		const seastar::shared_ptr<NodeCollection>& nodeCollection) :
		sessionConfiguration_(sessionConfiguration),
		nodeCollection_(nodeCollection),
		allConnections_(),
		waiters_(sessionConfiguration_->getMaxWaitersAfterConnectionsExhausted()),
		findIdleConnectionTimerIsRunning_(false) { }

	/** Make a new ready-to-use connection and return it with an idle stream */
	seastar::future<seastar::lw_shared_ptr<Connection>, ConnectionStream>
		ConnectionPool::makeConnection() {
		seastar::promise<seastar::lw_shared_ptr<Connection>, ConnectionStream> promise;
		auto future = promise.get_future();
		auto self = shared_from_this();
		seastar::do_with(
			std::move(promise), std::move(self), static_cast<std::size_t>(0U),
			[] (auto& promise, auto& self, auto& count) {
			return seastar::repeat([&promise, &self, &count] {
				// choose one node and create connection
				auto node = self->nodeCollection_->chooseOneNode();
				auto connection = seastar::make_lw_shared<Connection>(
					self->sessionConfiguration_, node);
				// initialize connection
				return connection->ready().then([&promise, &self, node, connection] {
					// initialize connection success
					auto stream = connection->openStream();
					if (!stream.isValid()) {
						promise.set_exception(LogicException(CQL_CODEINFO,
							"open stream form a newly created connection failed"));
					} else {
						self->nodeCollection_->reportSuccess(node);
						self->allConnections_.emplace_back(connection);
						promise.set_value(std::move(connection), std::move(stream));
					}
					return seastar::stop_iteration::yes;
				}).handle_exception([&promise, &self, &count, node] (std::exception_ptr ex) {
					// initialize connection failed, try next node until all tried
					self->nodeCollection_->reportFailure(node);
					if (++count >= self->nodeCollection_->getNodesCount()) {
						promise.set_exception(ex);
						return seastar::stop_iteration::yes;
					} else {
						return seastar::stop_iteration::no;
					}
				});
			});
		});
		return future;
	}

	/** Timer used to find idle connection and feed waiters */
	void ConnectionPool::findIdleConnectionTimer() {
		if (findIdleConnectionTimerIsRunning_) {
			return;
		}
		findIdleConnectionTimerIsRunning_ = true;
		auto self = shared_from_this();
		seastar::do_with(std::move(self), [] (auto& self) {
			return seastar::repeat([&self] {
				return seastar::sleep(std::chrono::milliseconds(1)).then([&self] {
					while (!self->waiters_.empty()) {
						auto result = self->tryGetConnection();
						if (result.first.get() != nullptr) {
							self->waiters_.pop().set_value(
								std::move(result.first), std::move(result.second));
						} else {
							break;
						}
					}
					return self->waiters_.empty() ?
						seastar::stop_iteration::yes :
						seastar::stop_iteration::no;
				});
			}).finally([&self] {
				self->findIdleConnectionTimerIsRunning_ = false;
			});
		});
	}

	/** Timer used to drop idle connections */
	void ConnectionPool::dropIdleConnectionTimer() {
		throw NotImplementedException(CQL_CODEINFO, "not implemented");
	}
}
