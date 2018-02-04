#!/bin/bash

COMPRESSORS=$*
BWINFOCP=$(basename -s .x $((ls bin & (ls bin | awk '$0="t"$0')) | sort | uniq -d | cut -c 1 --complement))
#get information about operating system
CPUINFO=$(grep 'model name' /proc/cpuinfo | uniq | cut -d: -f2 | awk '{$1=$1};1')
MEMINFO=$(grep 'MemTotal' /proc/meminfo | cut -d: -f2 | awk '{$1=$1};1')
OSINFO=$(uname -s -v -m | awk '{$1=$1};1')

cat <<'THEEND'
\documentclass{article}
\usepackage[utf8]{inputenc}
\usepackage[T1]{fontenc}
\usepackage{amsmath}
\usepackage{amssymb}
\usepackage{booktabs}
\usepackage{mathtools}
\usepackage{multirow}
\usepackage{pgfplots}
\usepackage{pgfplotstable}
\usepackage{threeparttable}
\usepackage{url}
\pgfplotsset{compat=1.9}

THEEND

#create data tables
echo "\\pgfplotstableread{"
cat result.dat
echo "}\datatable"
#this table is only used for the tunneled bwt
echo "\\pgfplotstableread{"
cat bwinfo.dat
echo "}\bwinfo"

cat <<'THEEND'

%%%% CREATE DATA TABLES %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%% TABLE 1: COMPRESSION %%%%
\newcommand{\compressiontable}{
\begin{threeparttable}
\renewcommand{\arraystretch}{1.25}
\setlength{\tabcolsep}{.25em}
\scriptsize
%end of format issues
\pgfplotstabletypeset[
	every head row/.style={
		before row={\toprule},
		after row={\midrule}},
	every last row/.style={
		after row={\bottomrule}},
THEEND
echo -n "	columns={file"
for method in $COMPRESSORS ; do
	echo -n ",$method-bps";
done
echo "}"
echo "	,columns/file/.style={string type,column type=l,column name=File}"
for method in $COMPRESSORS ; do
	echo "	,columns/$method-bps/.style={string type,column type=c,column name=\rotatebox{90}{\texttt{$method}}}";
done
cat <<'THEEND'
	,row predicate/.code={
		\pgfplotstablegetelem{\pgfplotstablerow}{bps-best}\of{\datatable}
		\edef\rowindex{\pgfplotstablerow}
		\edef\colname{\pgfplotsretval-bps}
		\edef\setstyles{\noexpand\pgfplotstableset{
			every row \rowindex\noexpand\space column \colname/.style={
				postproc cell content/.append style={
					/pgfplots/table/@cell content/.add={$\noexpand\bf}{$}
				},
			}
		}}\setstyles
	}]\datatable
\end{threeparttable}
}

%%%% TABLE 2: ENCODING SPEEDS %%%%

\newcommand{\encodespeedtable}{
\begin{threeparttable}
\renewcommand{\arraystretch}{1.25}
\setlength{\tabcolsep}{.25em}
\scriptsize
\pgfplotstabletypeset[
	every head row/.style={
		before row={\toprule},
		after row={\midrule}},
	every last row/.style={
		after row={\bottomrule}},
THEEND
echo -n "	columns={file"
for method in $COMPRESSORS ; do
	echo -n ",$method-encode-speed";
done
echo "}"
echo "	,columns/file/.style={string type,column type=l,column name=File}";
for method in $COMPRESSORS ; do
	echo "	,columns/$method-encode-speed/.style={string type,column type=r,column name=\rotatebox{90}{\texttt{$method}}}";
done
cat <<'THEEND'
	,row predicate/.code={
		\pgfplotstablegetelem{\pgfplotstablerow}{encode-speed-best}\of{\datatable}
		\edef\rowindex{\pgfplotstablerow}
		\edef\colname{\pgfplotsretval-encode-speed}
		\edef\setstyles{\noexpand\pgfplotstableset{
			every row \rowindex\noexpand\space column \colname/.style={
				postproc cell content/.append style={
					/pgfplots/table/@cell content/.add={$\noexpand\bf}{$}
				},
			}
		}}\setstyles
	}]\datatable
\end{threeparttable}
}

%%%% TABLE 3: DECODING SPEEDS %%%%

\newcommand{\decodespeedtable}{
\begin{threeparttable}
\renewcommand{\arraystretch}{1.25}
\setlength{\tabcolsep}{.25em}
\scriptsize
\pgfplotstabletypeset[
	every head row/.style={
		before row={\toprule},
		after row={\midrule}},
	every last row/.style={
		after row={\bottomrule}},
THEEND
echo -n "	columns={file"
for method in $COMPRESSORS ; do
	echo -n ",$method-decode-speed";
done
echo "}"
echo "	,columns/file/.style={string type,column type=l,column name=File}";
for method in $COMPRESSORS ; do
	echo "	,columns/$method-decode-speed/.style={string type,column type=r,column name=\rotatebox{90}{\texttt{$method}}}";
done
cat <<'THEEND'
	,row predicate/.code={
		\pgfplotstablegetelem{\pgfplotstablerow}{decode-speed-best}\of{\datatable}
		\edef\rowindex{\pgfplotstablerow}
		\edef\colname{\pgfplotsretval-decode-speed}
		\edef\setstyles{\noexpand\pgfplotstableset{
			every row \rowindex\noexpand\space column \colname/.style={
				postproc cell content/.append style={
					/pgfplots/table/@cell content/.add={$\noexpand\bf}{$}
				},
			}
		}}\setstyles
	}]\datatable
\end{threeparttable}
}

%%%% TABLE 4: ENCODE MEMPEAKS %%%%

\newcommand{\encodemempeaktable}{
\begin{threeparttable}
\renewcommand{\arraystretch}{1.25}
\setlength{\tabcolsep}{.25em}
\scriptsize
\pgfplotstabletypeset[
	every head row/.style={
		before row={\toprule},
		after row={\midrule}},
	every last row/.style={
		after row={\bottomrule}},
THEEND
echo -n "	columns={file"
for method in $COMPRESSORS ; do
	echo -n ",$method-encode-membps";
done
echo "}"
echo "	,columns/file/.style={string type,column type=l,column name=File}";
for method in $COMPRESSORS ; do
	echo "	,columns/$method-encode-membps/.style={string type,column type=r,column name=\rotatebox{90}{\texttt{$method}}}";
done
cat <<'THEEND'
	,row predicate/.code={
		\pgfplotstablegetelem{\pgfplotstablerow}{encode-membps-best}\of{\datatable}
		\edef\rowindex{\pgfplotstablerow}
		\edef\colname{\pgfplotsretval-encode-membps}
		\edef\setstyles{\noexpand\pgfplotstableset{
			every row \rowindex\noexpand\space column \colname/.style={
				postproc cell content/.append style={
					/pgfplots/table/@cell content/.add={$\noexpand\bf}{$}
				},
			}
		}}\setstyles
	}]\datatable
\end{threeparttable}
}

%%%% TABLE 5: DECODE MEMPEAKS %%%%

\newcommand{\decodemempeaktable}{
\begin{threeparttable}
\renewcommand{\arraystretch}{1.25}
\setlength{\tabcolsep}{.25em}
\scriptsize
\pgfplotstabletypeset[
	every head row/.style={
		before row={\toprule},
		after row={\midrule}},
	every last row/.style={
		after row={\bottomrule}},
THEEND
echo -n "	columns={file"
for method in $COMPRESSORS ; do
	echo -n ",$method-decode-membps";
done
echo "}"
echo "	,columns/file/.style={string type,column type=l,column name=File}";
for method in $COMPRESSORS ; do
	echo "	,columns/$method-decode-membps/.style={string type,column type=r,column name=\rotatebox{90}{\texttt{$method}}}";
done
cat <<'THEEND'
	,row predicate/.code={
		\pgfplotstablegetelem{\pgfplotstablerow}{decode-membps-best}\of{\datatable}
		\edef\rowindex{\pgfplotstablerow}
		\edef\colname{\pgfplotsretval-decode-membps}
		\edef\setstyles{\noexpand\pgfplotstableset{
			every row \rowindex\noexpand\space column \colname/.style={
				postproc cell content/.append style={
					/pgfplots/table/@cell content/.add={$\noexpand\bf}{$}
				},
			}
		}}\setstyles
	}]\datatable
\end{threeparttable}
}

%%%% TABLE 6: TUNNELED BWT ESTIMATORS QUALITY %%%%

\pgfplotstableset{
	create on use/tunneling-potential/.style={
		create col/expr={
			\thisrow{tbwzip-expectedtbwtgrossbenefitsize} / \thisrow{bwzip-bwtencodingsize} * 100
		}
	}
THEEND
for bwcp in $BWINFOCP ; do
echo ",
	create on use/$bwcp-model-fit/.style={
		create col/expr={
			max(0,min(
			(1 + \thisrow{t$bwcp-auxencodingsize} / (\thisrow{$bwcp-bwtencodingsize} - \thisrow{t$bwcp-tbwtencodingsize} - \thisrow{t$bwcp-auxencodingsize}))
			,
			(1 + \thisrow{t$bwcp-expectedauxtaxsize} / (\thisrow{t$bwcp-expectedtbwtgrossbenefitsize} - \thisrow{t$bwcp-expectedauxtaxsize}))
			)) / max(
			(1 + \thisrow{t$bwcp-auxencodingsize} / (\thisrow{$bwcp-bwtencodingsize} - \thisrow{t$bwcp-tbwtencodingsize} - \thisrow{t$bwcp-auxencodingsize}))
			,
			(1 + \thisrow{t$bwcp-expectedauxtaxsize} / (\thisrow{t$bwcp-expectedtbwtgrossbenefitsize} - \thisrow{t$bwcp-expectedauxtaxsize}))
			)
			* 100
		}
	}";
done
cat <<'THEEND'
}


\newcommand{\tbwtestimatortable}{
\begin{threeparttable}
\renewcommand{\arraystretch}{1.25}
\setlength{\tabcolsep}{.5em}
\scriptsize
%end of format issues
\pgfplotstabletypeset[
	every head row/.style={
		before row={\toprule},
		after row={\midrule}},
	every last row/.style={
		after row={\bottomrule}},
THEEND
echo -n "	columns={file,tunneling-potential";
for bwcp in $BWINFOCP ; do
	echo -n ",$bwcp-model-fit";
done
echo "},";
cat <<'THEEND'
	columns/file/.style={string type,column type=l,column name=File},
	columns/tunneling-potential/.style={column type=c,column name=\rotatebox{90}{\texttt{tunneling\newline potential}},
		postproc cell content/.append style={
			/pgfplots/table/@cell content/.add={}{~\%},
		}
	},
THEEND
for bwcp in $BWINFOCP ; do
	echo "	columns/$bwcp-model-fit/.style={column type=c,";
	echo "		column name=\rotatebox{90}{\texttt{$bwcp-model-fit}},";
	echo "		postproc cell content/.append style={";
	echo "			/pgfplots/table/@cell content/.add={}{~\%},";
	echo "		}";
	echo "	},";
done
cat <<'THEEND'
	fixed,fixed zerofill,precision=2]\bwinfo
\end{threeparttable}
}
THEEND

#### END OF DATA TABLES, START OF .TEX-FILE ###################################

cat <<'THEEND'

\title{Compression Benchmark Results}
\author{}
\begin{document}
\maketitle

%display compression data
\section{Compression}
This section shows compression rates achieved, measured in bits per symbol.%
\footnote{Example: let \texttt{f.txt} be a file with size $2$ MB, \texttt{f.comp} be the compressed
	file of \texttt{f.txt} with size $1$ MB, then the respective bits per symbol of the used compression 
	algorithm applied to \texttt{f.txt} is $\frac{8 \cdot 1 \text{ MB}}{2 \text{ MB}} = 4$ bps.}%

\begin{table}[ht!]
\centering
\compressiontable
\end{table}

%display coding speeds
\clearpage
\section{Coding Speeds}
This section shows speeds for encoding and decoding, measured in $\frac{\text{MB}}{\text{s}}$.%
\footnote{Example: let \texttt{f.txt} be a file with size $2$ MB, which is encoded/decoded
	in $1$ second. The respective encoding/decoding speed thus is 
$\frac{2 \text{ MB}}{1 \text{ s}} = 2 ~ \frac{\text{MB}}{\text{s}}$.}%

\subsection{Encoding}
\begin{table}[ht!]
\centering
\encodespeedtable
\end{table}

\subsection{Decoding}
\begin{table}[ht!]
\centering
\decodespeedtable
\end{table}

%display memory peak data
\clearpage
\section{Memory Peaks}
This section shows peaks of memory usage during construction, measured in bits per symbol.

\subsection{Encoding}
\begin{table}[ht!]
\centering
\encodemempeaktable
\end{table}

\subsection{Decoding}
\begin{table}[ht!]
\centering
\decodemempeaktable
\end{table}

%display estimator quality of tunneled BWT
\clearpage
\section{Tunneled BWT estimator quality}
This section shows quality measurements for estimations done in the tunneled BWT.
To this end, the gross-net-benefit ratio is defined as the fraction of the gross benefit
(benefit not including the aux-encoding) and net benefit (benefit minus size of the encoding for aux).
Use is motivated by two facts:
\begin{itemize}
\item	Small variations in the aux-encoding can be tolerated if the benefit is orders of magnitude bigger.
\item	The ratio is not affected by the efficiency of a compressor, making it nicely comparable.
\end{itemize}
To get a measure how well the theoretical model fits onto the given compressor, the model-fit is defined as
the minimax-distance of the gross-net-benefit ratio of both model and compressor, i.e.\
$\frac{\min\{\widehat{\mathsf{gnb}},\mathsf{gnb}\}}{\max\{\widehat{\mathsf{gnb}},\mathsf{gnb}\}}$.

Furthermore the tunneling potential is defined as the theoretical gross-net-benefit divided by
the size of the same  \texttt{bwz}-encoded text, indicating how much benefit theoretically should be possible.

\begin{table}[ht!]
\centering
\tbwtestimatortable
\end{table}

%display system information
\clearpage
\section{System Information}
Following list shows information about processor, installed memory and operating system.
\begin{table}[ht!]
\centering
\renewcommand{\arraystretch}{1.5}
\renewcommand{\tabcolsep}{1em}
\begin{tabular}{r|l}
THEEND
echo "CPU & \\verb|$CPUINFO| \\\\"
echo "Memory & \\verb|$MEMINFO| \\\\"
echo "OS Info & \\verb|$OSINFO| \\\\"
cat <<'THEEND'
\end{tabular}
\end{table}

\end{document}
THEEND
