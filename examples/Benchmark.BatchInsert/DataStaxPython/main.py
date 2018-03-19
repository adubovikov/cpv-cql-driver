#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from cassandra.cluster import Cluster
from cassandra.query import SimpleStatement, BatchStatement
from cassandra import ConsistencyLevel
import time

LOOP_COUNT=1000
INSERT_COUNT=100

if __name__ == "__main__":
	cluster = Cluster(["127.0.0.1"], port=9043)
	session = cluster.connect()
	session.execute("drop keyspace if exists benchmark_ks")
	session.execute("create keyspace benchmark_ks with replication = " +
		"{ 'class': 'SimpleStrategy', 'replication_factor': 1 }")
	session.execute("create table benchmark_ks.my_table (id int primary key, name text)")

	start=time.time()
	for i in range(LOOP_COUNT):
		insertStatement = SimpleStatement(
			"insert into benchmark_ks.my_table (id, name) values (%s, %s)",
			consistency_level=ConsistencyLevel.QUORUM)
		insertBatch = BatchStatement()
		for j in range(INSERT_COUNT):
			insertBatch.add(insertStatement, (i*INSERT_COUNT+j, "name"))
		session.execute(insertBatch)
	print("used seconds:", time.time()-start)
