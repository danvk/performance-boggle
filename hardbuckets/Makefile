test: buckets_test
	./buckets_test

BUCKETS=buckets.o $(BOGGLE)
buckets_test: buckets_test.o $(BUCKETS)
bucket_boggle: bucket_boggle.o $(BUCKETS)
bucket_descent: bucket_descent.o $(BUCKETS)
bucket_trie: bucket_trie.o $(BUCKETS)
buckets.o: buckets.h buckets.cc boggler.h trie.h
