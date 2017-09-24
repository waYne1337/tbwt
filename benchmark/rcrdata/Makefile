include rcrtestcases.config

all:	$(RCRFILES)

#generate texts from the silesia corpus
$(SILESIA):
	curl http://sun.aei.polsl.pl/~sdeor/corpus/$(@F).bz2 -o $@.bz2
	bunzip2 -f $@.bz2

#download texts from the pizza & chili corpus
$(PIZZACHILI):
	$(eval fdir := \
		$(if $(findstring $(basename $(@F)),sources),code,\
		$(if $(findstring $(basename $(@F)),pitches),music,\
		$(if $(findstring $(basename $(@F)),proteins),protein,\
		$(if $(findstring $(basename $(@F)),dna),dna,\
		$(if $(findstring $(basename $(@F)),english),nlang,\
		$(if $(findstring $(basename $(@F)),dblp.xml),xml,\
			$(error unknown pizza chili category of $(@F) ))))))))
	curl http://pizzachili.dcc.uchile.cl/texts/$(fdir)/$(@F).gz -o $@.gz
	gunzip -f $@.gz

#generate repetitive texts
$(REPETITIVE):
	curl http://pizzachili.dcc.uchile.cl/repcorpus/real/$(@F).gz -o $@.gz
	gunzip -f $@.gz

clean:
	rm -f $(RCRFILES)
