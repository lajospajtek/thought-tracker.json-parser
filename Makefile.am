SUBDIRS = src

COVERDIR = $(top_srcdir)/metrics/coverage

.PHONY: coverage-campaign doc

doc:
	cd $(top_srcdir); doxygen

coverage-campaign: $(COVERDIR)
	$(MAKE) clean && \
	$(MAKE) coverage-clean && \
	$(MAKE) CXXFLAGS="-O0 -ggdb -fprofile-arcs -ftest-coverage" check && \
	$(MAKE) coverage && \
	$(MAKE) coverage-report && \
	$(MAKE) coverage-history

$(COVERDIR):
	if [ ! -d $(COVERDIR) ]; then mkdir -p $(COVERDIR); fi

coverage-check:
	$(MAKE) CXXFLAGS="-O0 -ggdb -fprofile-arcs -ftest-coverage" check

coverage-clean:
	-rm -f *.info */unit/*.gcda */.libs/*.gcda $(COVERDIR)/*.info

coverage:
	@lcovparams=""; \
	for p in $(SUBDIRS); do \
	  nbtest=`ls -l $$p/unit/*.gcda | wc -l`; \
	  if [ $${nbtest} -gt 0 ]; then \
	    lcov --output-file $$p.full.info --capture --directory $$p/unit --test-name $$p; \
	    lcov --output-file $$p.tmp.info --extract $$p.full.info "*/$$p/*.*" ; \
	    lcov --output-file $(COVERDIR)/$$p.info --remove $$p.tmp.info "*unit*" ; \
	    lcovparams+=" --add-tracefile $(COVERDIR)/$$p.info"; \
	  fi; \
	done; \
	tagrel=`echo "$Name:  $ " | awk '{print $$2}'`; \
	lcov --test-name "$${tagrel}" --output-file $(COVERDIR)/all.info $${lcovparams};

coverage-report: 
	@tagrel=`echo "$Name:  $ " | awk '{print $$2}'`; \
	genhtml --show-details --title "$$tagrel" \
	  --output-directory $(COVERDIR)/html $(COVERDIR)/all.info

# generate test coverage percentage and merge with history

CoverageHistoryFile = $(COVERDIR)/all.history.csv
GenCoverageHistoryFile=$(CoverageHistoryFile).`date +\%F`

coverage-history: coverage-history-merge
	echo "history Done"

coverage-history-merge: coverage-history-prepare
	@tagrel=`echo "$Name:  $ " | awk '{print $$2}'`; \
	head -1 $(CoverageHistoryFile) | \
	  awk "{print \$$1\":`date +%F`\"}" > $(GenCoverageHistoryFile); \
	head -2 $(CoverageHistoryFile) | tail -1 | \
	  awk "{print \$$1\":$$tagrel\"}" >> $(GenCoverageHistoryFile); \
	sort common.tmp.csv deleted.tmp.csv new.tmp.csv >> $(GenCoverageHistoryFile); \
	echo "merge Done"

coverage-history-prepare: coverage-gen
	@initline="`head -1 $(CoverageHistoryFile) | \
	  sed 's/\[File\]//' | sed 's/:\([-a-zA-Z0-9]*\)/:-1/g'`"; \
	tail -n +3 $(CoverageHistoryFile) > history.tmp.csv; \
	join -j1 -t ":" history.tmp.csv release.tmp.txt > common.tmp.csv; \
	join -j1 -v1 -t ":" history.tmp.csv release.tmp.txt | \
	  awk '{print $$0":-1"}' > deleted.tmp.csv; \
	join -j1 -v2 -t ":" history.tmp.csv release.tmp.txt | \
	  awk "BEGIN{FS=\":\"}{print \$$1\"$$initline:\"\$$2}" > new.tmp.csv; \
	echo "prepare Done"

coverage-history-init: coverage-gen
	@tagrel=`echo "$Name:  $ " | awk '{print $$2}'`; \
	echo "[File]:`date +%F`" > $(CoverageHistoryFile); \
	echo "[File]:$$tagrel" >> $(CoverageHistoryFile); \
	cat release.tmp.txt >> $(CoverageHistoryFile)

coverage-gen:
	lcov --list $(COVERDIR)/all.info | sed -e s?$(top_srcdir)/src/?? | \
	  sort | awk '{printf("%s%.2f\n",$$1,100*$$2/$$4)}' > release.tmp.txt; \
	echo "stat Done"

coverage-history-clean:
	rm -f history.tmp.csv release.tmp.txt \
	  common.tmp.csv deleted.tmp.csv new.tmp.csv
