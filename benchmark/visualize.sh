#!/bin/bash

COMPRESSORS=$*

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
cat estquality.dat
echo "}\estimatorquality"

cat <<'THEEND'

%%%% CREATE DATA TABLES %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%% TABLE 1: COMPRESSION %%%%
\newcommand{\compressiontable}{
\begin{threeparttable}
\renewcommand{\arraystretch}{1.25}
\setlength{\tabcolsep}{.5em}
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
	echo "	,columns/$method-bps/.style={string type,column type=r,column name=\texttt{$method}}";
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
\setlength{\tabcolsep}{.5em}
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
	echo "	,columns/$method-encode-speed/.style={string type,column type=r,column name=\texttt{$method}}";
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
\setlength{\tabcolsep}{.5em}
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
	echo "	,columns/$method-decode-speed/.style={string type,column type=r,column name=\texttt{$method}}";
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
\setlength{\tabcolsep}{.5em}
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
	echo "	,columns/$method-encode-membps/.style={string type,column type=r,column name=\texttt{$method}}";
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
\setlength{\tabcolsep}{.5em}
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
	echo "	,columns/$method-decode-membps/.style={string type,column type=r,column name=\texttt{$method}}";
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
\newcommand{\tbwtestimatortable}{
\begin{threeparttable}
\renewcommand{\arraystretch}{1.25}
\setlength{\tabcolsep}{1.5em}
%end of format issues
\pgfplotstabletypeset[
	every head row/.style={
		before row={\toprule},
		after row={\midrule}},
	every last row/.style={
		after row={\bottomrule}},
	columns/file/.style={string type,column type=l,column name=File},
	columns/aux-rel-error/.style={column type=r,column name={$\displaystyle \frac{\hat{\mathsf{aux}}-\mathsf{aux}}{\mathsf{aux}}$}},
	columns/size-benefit-rel-error/.style={column type=r,column name={$\displaystyle \frac{\hat{\mathsf{benefit}}-\mathsf{benefit}}{\mathsf{benefit}}$}},
	fixed,fixed zerofill,precision=2
	]\estimatorquality
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
This section shows compression rates achieved, measured in bits per symbol of the original text%
\footnote{Example: let \texttt{f.txt} be a file with size $2$ MB, \texttt{f.comp} be the compressed
	file of \texttt{f.txt} with size $1$ MB, then the respective bits per symbol of the used compression 
	algorithm applied to \texttt{f.txt} is $\frac{8 \cdot 1 \text{ MB}}{2 \text{ MB}} = 4$ bps.}%
. Results can be found in Table \ref{tbl:compression}.

\begin{table}[ht!]
\centering
\compressiontable
\caption{Compression achieved by used methods, measured in bits per symbol.}
\label{tbl:compression}
\end{table}

%display coding speeds
\clearpage
\section{Coding Speeds}
This section shows speeds for encoding and decoding, measured in $\frac{\text{MB}}{\text{s}}$
 of the original input%
\footnote{Example: let \texttt{f.txt} be a file with size $2$ MB, which is encoded/decoded
	in $1$ second. The respective encoding/decoding speed thus is 
$\frac{2 \text{ MB}}{1 \text{ s}} = 2 ~ \frac{\text{MB}}{\text{s}}$.}%
. Results can be found in Tables \ref{tbl:encodespeeds} and \ref{tbl:decodespeeds}.

\subsection{Encoding}
\begin{table}[ht!]
\centering
\encodespeedtable
\caption{Encode speed of used methods, measured in MB per second.}
\label{tbl:encodespeeds}
\end{table}

\subsection{Decoding}
\begin{table}[ht!]
\centering
\decodespeedtable
\caption{Decode speed of used methods, measured in MB per second.}
\label{tbl:decodespeeds}
\end{table}

%display memory peak data
\clearpage
\section{Memory Peaks}
This section shows peaks of memory usage during construction, measured in bits per symbol.
The results can be found in Tables \ref{tbl:encodemempeak} and \ref{tbl:decodemempeak}.

\subsection{Encoding}
\begin{table}[ht!]
\centering
\encodemempeaktable
\caption{Memory peak of used methods during encoding, measured in bits per symbol.}
\label{tbl:encodemempeak}
\end{table}

\subsection{Decoding}
\begin{table}[ht!]
\centering
\decodemempeaktable
\caption{Memory peak of used methods during decoding, measured in bits per symbol.}
\label{tbl:decodemempeak}
\end{table}

%display estimator quality of tunneled BWT
\clearpage
\section{Tunneled BWT estimator quality}
This section shows quality measurements for estimations done in the tunneled BWT.
To this end, for any Observable $x \in \mathcal X$, we denote by $\hat{x}$ the estimator for
$x$. Furthermore, $\mathsf{aux}$ identifies the additional data structure required for tunneling,
while $\mathsf{benefit}$ identifies the whole benefit of tunneling compared to the normal BWT,
including the costs for $\mathsf{aux}$.

\begin{table}[ht!]
\centering
\tbwtestimatortable
\caption{Quality of estimators used in the Tunneled BWT. A positive value indicates that the estimator estimated too much,
	while a negative value indicates a too low estimate.}
\label{tbl:tbwtestquality}
\end{table}

\end{document}
THEEND
