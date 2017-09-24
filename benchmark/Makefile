include testcases.config
include compressors.config

#max runtime per command (used in timeout)
MAXTIME := 90m

all:	result.dat estquality.dat result.tex result.pdf 

install:	.installed

#install compressors
.installed:
	chmod a+x visualize.sh
	@for method in $(COMPRESSORS) ; do \
		chmod a+x cp/$$method.sh; \
		cp/$$method.sh i; \
	done
	> .installed

#execute benchmark with configured compressors
result.dat:	.installed $(addsuffix .sh,$(addprefix cp/,$(COMPRESSORS))) testcases.config compressors.config
	@echo -n "file" | tee result.dat;
	@for method in $(COMPRESSORS) ; do \
		echo -n " $$method-bps $$method-decode-speed $$method-decode-membps $$method-encode-speed $$method-encode-membps" >> result.dat; \
		echo -n "\t$$method-bps"; \
	done
	@echo " bps-best decode-speed-best decode-membps-best encode-speed-best encode-membps-best" >> result.dat
	@echo ""
	@for tcfile in $(TCFILES) ; do \
		tcname=$$(basename "$$tcfile" | tr '_' '-'); \
		echo -n $$tcname | tee -a result.dat; \
		tcfs=$$(stat -c%s $$tcfile); \
		bpsbest="file"; \
		mincompfs=0; \
		dsbest="file"; \
		mindtime=0; \
		dbpsbest="file"; \
		mindmem=0; \
		esbest="file"; \
		minetime=0; \
		ebpsbest="file"; \
		minemem=0; \
		for compressor in $(COMPRESSORS) ; do \
			echo -n "" > tmp/fcomp; \
			echo -n "" > tmp/fres; \
			estats=$$(timeout $(MAXTIME) time -f "%U %S %M" cp/$$compressor.sh c $$tcfile tmp/fcomp  2>&1 1>/dev/null ); \
			if [ $$? -ne 0 ] ; then \
				echo -n " 0 0 $$tcfs 0 $$tcfs" >> result.dat; \
				echo -n "\t[FAIL]"; \
				continue; \
			fi; \
			compfs=$$(stat -c%s tmp/fcomp); \
			dstats=$$(timeout $(MAXTIME) time -f "%U %S %M" cp/$$compressor.sh d tmp/fcomp tmp/fres  2>&1 1>/dev/null ); \
			if [ $$? -ne 0 ] ; then \
				echo -n " 0 0 $$tcfs 0 $$tcfs 0" >> result.dat; \
				echo -n "\t[FAIL]"; \
				continue; \
			fi; \
			cmp -s $$tcfile tmp/fres; \
			if [ $$? -ne 0 ] ; then \
				echo -n " 0 0 $$tcfs 0 $$tcfs 0" >> result.dat; \
				echo -n "\t[FAIL]"; \
				continue; \
			fi; \
			eusrtime=$$(echo $$estats | awk '{print $$1}'); \
			esystime=$$(echo $$estats | awk '{print $$2}'); \
			etime=$$(echo "scale=3;$$eusrtime+$$esystime+0.001" | bc); \
			emem=$$(echo $$estats | awk '{print $$3}'); \
			emem=$$(echo "$$emem*1024" | bc); \
			dusrtime=$$(echo $$dstats | awk '{print $$1}'); \
			dsystime=$$(echo $$dstats | awk '{print $$2}'); \
			dtime=$$(echo "scale=3;$$dusrtime+$$dsystime+0.001" | bc); \
			dmem=$$(echo $$dstats | awk '{print $$3}'); \
			dmem=$$(echo "$$dmem*1024" | bc); \
			bps=$$(echo "scale=3;$$compfs*8/$$tcfs" | bc | sed 's/^\./0./'); \
			dspeed=$$(echo "scale=2;$$tcfs/($$dtime*1024*1024)" | bc | sed 's/^\./0./'); \
			espeed=$$(echo "scale=2;$$tcfs/($$etime*1024*1024)" | bc | sed 's/^\./0./'); \
			emembps=$$(echo "scale=2;$$emem*8/$$tcfs" | bc | sed 's/^\./0./'); \
			dmembps=$$(echo "scale=2;$$dmem*8/$$tcfs" | bc | sed 's/^\./0./'); \
			etime=$$(echo "scale=0;$$etime*1000/1" | bc | sed 's/^\./0./'); \
			dtime=$$(echo "scale=0;$$dtime*1000/1" | bc | sed 's/^\./0./'); \
			echo -n " $$bps $$dspeed $$dmembps $$espeed $$emembps" >> result.dat; \
			echo -n "\t$$bps"; \
			rm tmp/fcomp; \
			rm tmp/fres; \
			if [ "$$mincompfs" -ge "$$compfs" ] || [ "$$mincompfs" -eq 0 ]; then \
				mincompfs="$$compfs"; \
				bpsbest=$$compressor; \
			fi; \
			if [ "$$mindmem" -ge "$$dmem" ] || [ "$$mindmem" -eq 0 ]; then \
				mindmem="$$dmem"; \
				dbpsbest=$$compressor; \
			fi; \
			if [ "$$minemem" -ge "$$emem" ] || [ "$$minemem" -eq 0 ]; then \
				minemem="$$emem"; \
				ebpsbest=$$compressor; \
			fi; \
			if [ "$$mindtime" -ge "$$dtime" ] || [ "$$mindtime" -eq 0 ]; then \
				mindtime="$$dtime"; \
				dsbest=$$compressor; \
			fi; \
			if [ "$$minetime" -ge "$$etime" ] || [ "$$minetime" -eq 0 ]; then \
				minetime="$$etime"; \
				esbest=$$compressor; \
			fi; \
		done; \
		echo " $$bpsbest $$dsbest $$dbpsbest $$esbest $$ebpsbest" >> result.dat; \
		echo ""; \
	done

#execute estimator quality benchmark for tunneled bwt
estquality.dat:	.installed testcases.config
	@echo "file aux-rel-error size-benefit-rel-error" | tee estquality.dat; \
	numtestcases=$$(echo "$(TCFILES)" | wc -w); \
	for tcfile in $(TCFILES) ; do \
		tcname=$$(basename "$$tcfile" | tr '_' '-'); \
		tbwzinfo=$$(bin/tbwzip.x -c -i $$tcfile tmp/fcomp); \
		tbwzsize=$$(stat -c%s tmp/fcomp); \
		bwzinfo=$$(bin/bwzip.x -c -i $$tcfile tmp/fcomp); \
		bwzsize=$$(stat -c%s tmp/fcomp); \
		rm tmp/fcomp; \
		numtunnels=$$(echo "$$tbwzinfo" | awk '{ if ($$2 == "tunneled" && $$3 == "block" && $$4 == "count") {s+=$$5}} END {print s}'); \
		if [ "$$numtunnels" -ne "0" ] ; then \
			estsizebenefit=$$(echo "$$tbwzinfo" | awk '{ if ($$2 == "expected" && $$3 == "size" && $$4 == "benefit") {s+=$$5}} END {print s}'); \
			estauxsize=$$(echo "$$tbwzinfo" | awk '{ if ($$2 == "expected" && $$3 == "aux" && $$4 == "size") {s+=$$5}} END {print s}'); \
			realauxsize=$$(echo "$$tbwzinfo" | awk '{ if ($$2 == "aux" && $$3 == "size") {s+=$$4}} END {print s}'); \
			realsizebenefit=$$(echo "$$bwzsize-$$tbwzsize" | bc); \
			relauxerror=$$(echo "scale=4;$$estauxsize/$$realauxsize - 1" | bc); \
			relbenerror=$$(echo "scale=4;$$estsizebenefit/$$realsizebenefit - 1" | bc); \
			echo "$$tcname $$relauxerror $$relbenerror" | tee -a estquality.dat; \
		else \
			echo "$$tcname 0 0" | tee -a estquality.dat; \
		fi; \
	done

#generate tex-file for visualization
result.tex:	.installed visualize.sh estquality.dat result.dat compressors.config
	./visualize.sh $(COMPRESSORS) > result.tex

#generate pdf-file from visualization file
result.pdf:	.installed result.tex
	cp result.tex tmp/result.tex
	pdflatex -draftmode -output-directory=tmp tmp/result.tex
	pdflatex -draftmode -output-directory=tmp tmp/result.tex
	pdflatex -output-directory=tmp tmp/result.tex
	mv -f tmp/result.pdf result.pdf
	rm -f tmp/result*

#replicate computational results
rcr:
	cd rcrdata;make
	cp rcrdata/rcrtestcases.config testcases.config
	cp rcrdata/rcrcompressors.config compressors.config
	make result.pdf

#clean up benchmark
clean:
	rm -f estquality.dat
	rm -f result.dat
	rm -f result.tex
	rm -f result.pdf
	rm -f tmp/*
	rm -f bin/*
