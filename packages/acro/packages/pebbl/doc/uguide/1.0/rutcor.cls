%         *****************************************
%         *              rutcor.clo               *
%         *         November 20, 1997             *
%         *****************************************
%
% This file extends the standard article class for
% the use with RUTCOR research reports. It includes
% the 12pt and twoside options.
%
% You should define the title, author, and abstract 
% using the declarations 
%      \title{...}, \author{...}, and \abstract{...}. 
% Special acknowledgements can be given by 
%      \acknowledgement{...}.
% Affiliations are supposed (although not necessary) to be specified,
% right after each authors name (inside the \author command), by the 
%      \affiliation{...}.
% Inside all of the above commands you may want to use \\ 
% to start a new line. Never leave an empty line in the scope 
% of the above commands, because that may cause an error.
%
% The actual RRR-number will be issued after submission, 
% in order of submissions. It will be specified by the declaration 
%      \rrr{...}.
% The names of authors should be respecified, without affiliations,
% for the inside title page by
%      \authornames{...}.
%
% All the above should come before the line
%      \begin{document}.
%  
% For an example see the   
%
%       rutcor-sample.tex  
% 
% file. 
%
% Modifed version of rutcor.sty.
% Modifications by Stefan H. Schmieta.
\NeedsTeXFormat{LaTeX2e}
\ProvidesFile{rutcor.cls}[1997/11/20]
\typeout{RUTCOR Research Report class -- version 1 (Nov 20, 1997)}
\LoadClass[12pt,twoside]{article}

% define larger pages
\setlength{\oddsidemargin}{0 pt}   % Left margin on odd-numbered pages.
\setlength{\evensidemargin}{0 pt}  % Left margin on even-numbered pages.
\setlength{\marginparwidth}{0 pt} % Width of marginal notes.
\setlength{\topmargin}{-0.25in}      % 
\setlength{\textheight}{8.5in}     % Height of text (including footnotes and figures)
\setlength{\textwidth}{6.5in}      % Width of text line.

% report number
\newcommand{\rrr}[1]{\gdef\@rrr{#1}}
\newcommand{\authornames}[1]{\gdef\@authornames{#1}}
\renewcommand{\abstract}[1]{\gdef\@abstract{#1}}

\rrr{??-97} %Default RRR-number
\date{\thismonth}          %Default is thismonth
\global\let\@authornames\@empty
\global\let\@abstract\@empty



% only month and year
\def\thismonth{\ifcase\month\or
  January\or February\or March\or April\or May\or June\or
  July\or August\or September\or October\or November\or December\fi 
  \hskip .5em \number\year}

% acknowledgement put on the bottom of titlepage (not specific to an author)
\newcounter{ack}
\newcommand{\acknowledgement}[1]{\stepcounter{ack}\gdef\@acknowledgement{#1}}
\def\@acknowledgement{}

% affiliation of authors
\newcommand{\affiliation}[1]{\thanks{#1}}

\long\def\@rrrtext#1{\insert\footins{\footnotesize
    \interlinepenalty\interfootnotelinepenalty 
    \splittopskip\footnotesep
    \splitmaxdepth \dp\strutbox \floatingpenalty \@MM
    \hsize\columnwidth \@parboxrestore
   \edef\@currentlabel{\csname p@footnote\endcsname\@thefnmark}
   \parindent 1em\noindent\hbox to 1em{\hss$^{\ }$}
   \rule{\z@}{2\footnotesep}\ignorespaces
      #1\strut}}

\renewcommand{\maketitle}{
\@rutcorcover
\@rutcortitle
}

% The title page
% Here we change the margin and textwidth back to what it was.
% Then the title is typeset, followed by the abstract.
% Acknowledgements are placed at the bottom
%
\newcommand{\@rutcortitle}{
\setcounter{footnote}{0} 

\setlength{\textheight}{8.5in}
\setlength{\textwidth}{6.5in}
\setlength{\linewidth}{\textwidth}
\setlength{\hsize}{\textwidth}
\setlength{\columnwidth}{\textwidth}
\setlength{\marginparwidth}{0in}
\setlength{\oddsidemargin}{0in}
\setlength{\evensidemargin}{0in}
\setlength{\footskip}{42pt}

\begin{titlepage}
\thispagestyle{empty}
\let\footnotesize\small\let\footnoterule\relax 
\null
\centerline{\textsc{\large Rutcor Research Report}}
\vskip 5pt
\centerline{\textsc{RRR \@rrr, \@date}}
\vfill
\begin{center}
\textsc{\LARGE\@title} \vskip 5em {\large \lineskip .75em
\begin{tabular}[t]{c}\@authornames 
\end{tabular}\par}
\end{center} \par
\vfill
\begin{center}
\parbox{5.5in}{\small {\bf Abstract.}\@abstract}
\end{center}
\vskip 1em
\ifcase \theack \else
\@rrrtext{\parbox[t]{6in}{
\rule{6in}{.2pt}
{\bf Acknowledgements}: \@acknowledgement}}
\fi\par
\end{titlepage}

\setcounter{footnote}{0} 
\def\thefootnote{\arabic{footnote}}
}


% The cover page
% We put the RRR logo and the RUTCOR address in the left margin
% and the rest is a standard title page with RRR number and 
% author affiliations as footnotes.
%
\newcommand{\@rutcorcover}
{
\setlength{\textheight}{9in}
\setlength{\textwidth}{5in}
\setlength{\linewidth}{\textwidth}
\setlength{\hsize}{\textwidth}
\setlength{\columnwidth}{\textwidth}
\setlength{\marginparwidth}{1.55in}
\setlength{\oddsidemargin}{1.55in}
\setlength{\evensidemargin}{2.5in}
\setlength{\footskip}{1.5in}
\begin{titlepage}
\thispagestyle{empty}
\samepage
\let\footnotesize\small\let\footnoterule\relax 
\def\thefootnote{\alph{footnote}}
%
\reversemarginpar% default is right margin, but we want the left one.
\mbox{}\marginpar{
\parbox[t]{1.5in}{
%
%   First the LOGO
%
{\LARGE \textbf{R}\hfill\textsc{u\hfill t\hfill c\hfill o\hfill r}}\\*[0.2in]
{\LARGE \textbf{R}\hfill\textsc{e\hfill s\hfill e\hfill a\hfill r\hfill c\hfill h}}\\*[0.2in]
{\LARGE\textbf{R}\hfill\textsc{e\hfill p\hfill o\hfill r\hfill t}}
%
\vskip 5.3in
%
% Contact information at the bottom
%
{\scriptsize\sffamily 
RUTCOR\\
Rutgers Center for\\
Operations Research\\
Rutgers University\\  
640 Bartholomew Road\\
Piscataway, New Jersey\\ 
08854-8003\\
Telephone:\hfill 732-445-3804\\
Telefax:\hfill 732-445-5472\\
Email:\hfill {\sf rrr@rutcor.rutgers.edu} \\
http://rutcor.rutgers.edu/$\sim$rrr\hfill
}
}%
% A vertical line to the right
%
\hspace*{0.05in}\rule[-8.35in]{.2pt}{8.50in}\hfill 
}
%
% Now the title
%
\par\vfill
\begin{center}
\vspace*{\stretch{2}}
\textsc{\LARGE\@title}\par
\vskip 5em
{
\large \lineskip .75em
\begin{tabular}[t]{c}
\@author 
\end{tabular}
\par
}
\vskip 5em
\textsc{RRR \@rrr, \@date}
\vspace*{\stretch{3}}
\end{center}
\vfill
\rule{3in}{.2pt}
\@thanks
\end{titlepage}
\clearpage
}

\pagestyle{empty}{
\gdef\@oddhead{{{\sc RRR} \@rrr}\hfil {\sc Page \thepage}}
\gdef\@evenhead{{\sc Page \thepage}\hfil {{\sc RRR} \@rrr}}
}
\endinput



























































