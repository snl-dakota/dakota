\newcommand\SANDreportVersionOnly{v2.0e}
\newcommand\SANDreportVersion{12/02/19 \SANDreportVersionOnly}
% Make our output messages more easily searched
\newcommand{\SAND@typeout}[1]{\typeout{SANDreport.cls: #1}}


\SAND@typeout{Using Sandia National Laboratories Technical Report class \SANDreportVersionOnly, www.cs.sandia.gov/SANDreport}


% What are we providing
\ProvidesClass{SANDreport}[\SANDreportVersion]
\ProvidesFile{SANDreport.cls}[\SANDreportVersion]

% We need LaTeX 2e or better
\NeedsTeXFormat{LaTeX2e}

\RequirePackage[letterpaper]{geometry}
\RequirePackage{scrbase}
\RequirePackage{tocbasic}
\RequirePackage{etoolbox}
\RequirePackage{ifthen}

% Variables
\newboolean{blankSAND}			% Mark blank pages
\newboolean{relaxedSAND}
\newboolean{strictSAND}
\newboolean{reportSAND}
\newboolean{SANDmainProvided}
\newboolean{SANDnumProvided}
\newboolean{SANDprintDateProvided}
\newboolean{SANDrePrintDateProvided}
\newboolean{SANDauthorProvided}
\newboolean{SANDsupersedProvided}
\newboolean{SANDdistributionProvided}
\newboolean{distCA}
\newboolean{distNM}


% Counters
\newcounter{SANDprintingCnt}
\setcounter{SANDprintingCnt}{1}
\newcounter{SANDreDistCnt}
\setcounter{SANDreDistCnt}{1}
  

\DeclareOption{strict}{
  \setboolean{strictSAND}{true}
}
\ifthenelse{\boolean{relaxedSAND} \and \boolean{strictSAND}}   {
  \ClassWarning{SANDreport.cls}{Both, "strict" and "relax" options given}
}{
}
\DeclareOption{report}{
  \SAND@typeout{Option "report": Using the report class as a base.}
  \setboolean{reportSAND}{true}
}
\DeclareOption{CA}{
  \ClassError{SANDreport.cls}{"CA" option is obsolete}{Use
    \@backslashchar begin{SANDdistribution}[CA] instead.}
}
\DeclareOption{NM}{
  \ClassError{SANDreport.cls}{"NM" option is obsolete}{Use
    \@backslashchar begin{SANDdistribution}[NM] instead.}
}



% Manage some options using KOMA's keyval wrappers
\DefineFamily{SAND}
\DefineFamilyMember{SAND}

%
% The legacy SANDreport template used a single OUO option. To maintain some compatibility but
% still provide some better functionality, we reuse this. The OUO content tree is a little
% obtuse. 
%
% We mark for two types of OUO: UCI and UCNI. Each of these needs a specific FOIA exemption number.
% For FOIA exemption 3 (Statutory Exemption), there are subcategories of markings for each of UCI and UCNI:
%
% UCI
% - Generic OUO
% - OUO with Export Control notice (Atomic Energy Act)
% - OUO with CRADA notice
% - OUO with Export Control EAR notice (Export Administration Act)
% - OUO with ITAR Export Control
%
% UCNI
% - Generic OUO (regular OUO notice with added UCNI notice)
% - OUO with CRADA (above plus CRADA notice - total of 3 notices)
% - OUO with ITAR Export Control notice (OUO/UCNI + ITAR export control notice - total of 3)
% - OUO with EAR Export Control notice (OUO/UCNI + EAR export control notice - total of 3)
% - OUO with DOE Export Control notice (OUO/UCNI + DOE Atomic Energy Act EC notice - total of 3)
%
% So we have two broad categories of OUO with a subcategory for FOIA exemption 3. We handle this with
% three template options:
% - OUO, which can take two values. The default is UCI but UCNI is also valid.
% - FOIA=n, where n is the FOIA exemption number. The default is 3 (Statutory Exemption)
% - FOIAsubcat=flag. The flag values are intended to depend on which FOIA exemption is chosen;
% currently only FOIA 3 has a need for this but it's possible that might change.
% For FOIA 3, the flag values are one of: ECDOE,CRADA,ECITAR,ECEAR,PATENT. This determines which additional notices
% get added to the cover page.
%

% Refer to the KOMA-Script guide, Chapter 12 (Basic Functions in scrbase), section 12.2 (Keys as
% attributes of Families and their Members) for the details on how the \Family* commands work

\FamilyNumericalKey{SAND}{OUO}[UCI]{SAND@OUOoption}{%
  {no}{0},{none}{0},{UUR}{0},{UCI}{1},{UCNI}{2}%
}
\FamilyNumericalKey{SAND}{FOIA}[6]{SAND@FOIAoption}{%
  {3}{3},{4}{4},{5}{5},{6}{6},{7}{7},{custom}{10}%
}
\FamilyNumericalKey{SAND}{FOIAsubcat}[none]{SAND@FOIAsubcatOption}{%
  {no}{0},{none}{0},{ECDOE}{1},{CRADA}{2},{ECEAR}{3},{ECITAR}{4},{PATENT}{5}%
}
\FamilyBoolKey{SAND}{SP}{@SAND@SPoption}
\FamilyNumericalKey{SAND}{ClassLevel}[U]{SAND@ClassLevelOption}{%
	{U}{0},{C}{1},{S}{2},{TS}{3}%
}
\FamilyNumericalKey{SAND}{ClassCat}[U]{SAND@ClassCatOption}{%
	{U}{0},{NSI}{1},{FRD}{2},{RD}{3}%
}
\FamilyNumericalKey{SAND}{ClassCaveat}[none]{SAND@ClassCaveatOption}{%
	{none}{0},{NF}{1},{S15}{2}%
}

%%%%%%%%%%%%
%%
%%  Other class options
%%

%  defines option "blank" and switch \if@SAND@marks@blank
%  If set, blank pages will be marked "This page intentionally left blank."
\newif\if@SAND@marks@blank
\FamilyBoolKey{SAND}{blank}{@SAND@marks@blank}

% defines option "justified" and switch \if@SAND@text@justified
%  If set, all body text will be fully-justified
\newif\if@SAND@text@justified
\FamilyBoolKey{SAND}{justified}{@SAND@text@justified}

% defines option "innertitle" and switch \if@SAND@innertitle
%  If set, an inner title page (listing all authors and their affiliations and the abstract) will be generated
\newif\if@SAND@innertitle
\FamilyBoolKey{SAND}{innertitle}{@SAND@innertitle}

% Add some familiar options for selecting base font sizes. The article and report classes don't use a
% fontsize=XXpt option the way scrartcl/scrreprt do, so people expecting to be able to change font sizes
% in the same way (from when SANDreport.cls was based on the native classes) will need these.
\def\SAND@text@fontsize{12pt}
\DeclareOption{8pt}{\def\SAND@text@fontsize{8pt}}
\DeclareOption{9pt}{\def\SAND@text@fontsize{9pt}}
\DeclareOption{10pt}{\def\SAND@text@fontsize{10pt}}
\DeclareOption{11pt}{\def\SAND@text@fontsize{11pt}}
\DeclareOption{12pt}{\def\SAND@text@fontsize{12pt}}
\DeclareOption{13pt}{\def\SAND@text@fontsize{13pt}}
\DeclareOption{14pt}{\def\SAND@text@fontsize{14pt}}

% pass through options to the base class
\PassOptionsToPackage{paper=letter,usegeometry}{typearea}
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{SANDreport}}
\FamilyProcessOptions{SAND} % this also does the equivalent of \ProcessOptions\relax

\ifthenelse{\boolean{reportSAND}}{
  \LoadClass
  [fontsize=\SAND@text@fontsize,
  twoside,
  listof=entryprefix,
  numbers=endperiod,
  parskip=half,
  titlepage=firstiscover,
  bibliography=toc]
  {scrreprt}
}{
  \LoadClass
  [fontsize=\SAND@text@fontsize,
  twoside,
  listof=entryprefix,
  sectionentrydots=yes,
  numbers=endperiod,
  parskip=half,
  titlepage=firstiscover,
  bibliography=toc]
  {scrartcl}
}

\RequirePackage{scrlayer}
\RequirePackage{scrlayer-scrpage}

% Required fonts
% The body text font in the MS Word SAND report templates is Garamond. Without commenting on the merits of
% that choice, getting a functional version of Garamond (with bold variants and math symbols)
% in a LaTeX installation is non-trivial. We will proceed with Times NR until it becomes necessary to
% change.
\RequirePackage[T1]{fontenc}
\RequirePackage{times}
\RequirePackage{helvet}
\RequirePackage{mathptmx}

% Required packages
\RequirePackage{xcolor}
\RequirePackage{array}
\RequirePackage{longtable}
\RequirePackage{colortbl}
\RequirePackage{booktabs}
\RequirePackage{xspace}
\RequirePackage{graphicx}
\RequirePackage{amsmath}
\RequirePackage[normalem]{ulem} %The normalem option stops ulem from redefining the \emph{} command 


\newtoggle{SAND@OUO}
\newtoggle{SAND@OUOtype@UCI}
\newtoggle{SAND@OUOtype@UCNI}

\newtoggle{SAND@FOIA@statutory}
\newtoggle{SAND@FOIA@proprietary}
\newtoggle{SAND@FOIA@privileged}
\newtoggle{SAND@FOIA@pii}
\newtoggle{SAND@FOIA@lawenforcement}
\newtoggle{SAND@FOIA@custom}

\newtoggle{SAND@FOIA@ECDOE}
\newtoggle{SAND@FOIA@CRADA}
\newtoggle{SAND@FOIA@ECEAR}
\newtoggle{SAND@FOIA@ECITAR}
\newtoggle{SAND@FOIA@PATENT}

\newtoggle{SAND@SP}

\newtoggle{SAND@CLASS}
\newtoggle{SAND@CLASS@CONFIDENTIAL}
\newtoggle{SAND@CLASS@SECRET}
\newtoggle{SAND@CLASS@TOPSECRET}
\newtoggle{SAND@CLASS@NSI}
\newtoggle{SAND@CLASS@FRD}
\newtoggle{SAND@CLASS@RD}
\newtoggle{SAND@CLASS@NF}
\newtoggle{SAND@CLASS@S15}

\newtoggle{SAND@OUOorCLASS}

%
% Figure out the markings required
% 
\ifcase\SAND@OUOoption
% 0 case, not OUO
\togglefalse{SAND@OUO}
\SAND@typeout{Selecting no OUO: \SAND@OUOoption}
\or
% 1 case, UCI
\toggletrue{SAND@OUO}
\toggletrue{SAND@OUOtype@UCI}
\SAND@typeout{Selecting UCI: \SAND@OUOoption}
\or
% 2 case, UCNI
\toggletrue{SAND@OUO}
\toggletrue{SAND@OUOtype@UCNI}
\SAND@typeout{Selecting UCNI: \SAND@OUOoption}
\else
% anything else, not OUO
\togglefalse{SAND@OUO}
\SAND@typeout{Selecting no OUO from else: \SAND@OUOoption}
\fi

\SAND@typeout{FOIA: \SAND@FOIAoption}
\ifcase\SAND@FOIAoption\relax % 0 case
\or\relax % 1 case
\or\relax % 2 case
% now it gets more interesting
\or\toggletrue{SAND@FOIA@statutory} % 3
\or\toggletrue{SAND@FOIA@proprietary} % 4
\or\toggletrue{SAND@FOIA@privileged} % 5
\or\toggletrue{SAND@FOIA@pii} % 6
\or\toggletrue{SAND@FOIA@lawenforcement} % 7
\or\relax\or\relax % 8,9
\or\toggletrue{SAND@FOIA@custom} % 10
\fi

\SAND@typeout{FOIAsubcat: \SAND@FOIAsubcatOption}
\ifcase\SAND@FOIAsubcatOption\relax % 0 case
\or\toggletrue{SAND@FOIA@ECDOE}
\or\toggletrue{SAND@FOIA@CRADA}
\or\toggletrue{SAND@FOIA@ECEAR}
\or\toggletrue{SAND@FOIA@ECITAR}
\or\toggletrue{SAND@FOIA@PATENT}
\fi

\if@SAND@SPoption
\SAND@typeout{SPoption: true}
\toggletrue{SAND@SP}
\fi

\SAND@typeout{Classification level: \SAND@ClassLevelOption}
\ifcase\SAND@ClassLevelOption\relax
\togglefalse{SAND@CLASS}
\or
\toggletrue{SAND@CLASS}
\toggletrue{SAND@CLASS@CONFIDENTIAL}
\or
\toggletrue{SAND@CLASS}
\toggletrue{SAND@CLASS@SECRET}
\or
\toggletrue{SAND@CLASS}
\toggletrue{SAND@CLASS@TOPSECRET}
\else
\togglefalse{SAND@CLASS}
\fi

\SAND@typeout{Classification category: \SAND@ClassCatOption}
\ifcase\SAND@ClassCatOption\relax
\togglefalse{SAND@CLASS@NSI}
\or
\toggletrue{SAND@CLASS@NSI}
\or
\toggletrue{SAND@CLASS@FRD}
\or
\toggletrue{SAND@CLASS@RD}
\fi

\SAND@typeout{Classification caveat: \SAND@ClassCaveatOption}
\ifcase\SAND@ClassCaveatOption\relax
\togglefalse{SAND@CLASS@NF}
\or
\toggletrue{SAND@CLASS@NF}
\or
\toggletrue{SAND@CLASS@S15}
\fi

\iftoggle{SAND@OUO}{\toggletrue{SAND@OUOorCLASS}}{}
\iftoggle{SAND@CLASS}{\toggletrue{SAND@OUOorCLASS}}{}

% ******************************************************************************
% The commands to specify the report markings.
%

% Printed on the cover and title page below the SANDIA REPORT number
\newcommand{\SAND@marks@releaseType}{}
\newcommand{\SANDmarkReleaseUUR}{\renewcommand{\SAND@marks@releaseType}{Unclassified Unlimited Release}}

% Printed on the cover page
\newcommand{\SAND@marks@coverVar}{Approved for public release; further dissemination unlimited.} % default

% Header/footer mark
\newcommand{\SAND@marks@headfoot}{\relax}

% Implement "this page intentionally left blank" if the user asks for it
\if@SAND@marks@blank
\SAND@typeout{Turning on blank page marking.}
\def\emptypage@emptypage{%
  \hbox{}\vspace*{\fill}
  \begin{center}
    This page intentionally left blank.
  \end{center}
  \vspace{\fill}\newpage%
}%
\def\cleardoublepage{%
  \clearpage%
  \if@twoside%
  \ifodd\c@page%
  % no-op
  \else\emptypage@emptypage%
  \fi% odd page
  \fi% two side
}%
\fi

% What kind of justification will we use
\if@SAND@text@justified
\SAND@typeout{Using fully-justified body text.}
\newcommand{\SAND@text@justification}{} % default
\else
\SAND@typeout{Using ragged-right body text.}
\newcommand{\SAND@text@justification}{\raggedright}
\fi

\geometry{total={6.5in,9in},margin=1in}


% Time to start
\AtBeginDocument{

  % Do some usage checking
  \ifthenelse{\boolean{SANDnumProvided}}   {
  }{
    \ClassError{SANDreport.cls}{\@backslashchar SANDnum
      not provided}{Insert \@backslashchar SANDnum in the preamble
      of your document}
  }
  \ifthenelse{\boolean{SANDauthorProvided}}   {
  }{
    \ClassError{SANDreport.cls}{\@backslashchar SANDauthor
      not provided}{Insert \@backslashchar SANDauthor in the preamble
      of your document}
  }
  \ifthenelse{\boolean{SANDprintDateProvided}}   {
  }{
    \ClassError{SANDreport.cls}{\@backslashchar SANDprintDate
      not provided}{Insert \@backslashchar SANDprintDate in the
      preamble of your document}
  }
  
  %CTAN suggests using the subcaption package instead of the subfigure or subfig packages to create
  %subfigures within figures.  The subcaption package automatically sets subcaption font sizes to be too small, 
  %so we override the subcaption font size.
  \@ifpackageloaded{subcaption}{\captionsetup[sub]{size=footnotesize}}{}

  % I'm not sure why this is necessary. Without it, there are undefined control sequences in colortbl
  % which pop up in any use of \tabular. It's not effective if not in the source document (or here in
  % AtBeginDocument) so it appears that something prior to this is undoing the colortbl setup. Some
  % kind of weird interaction between KOMA-Script and colortbl that I gave up trying to track down - this
  % at least works around the problem. - pwidene@sandia.gov
  \arrayrulecolor{black}\doublerulesepcolor{black}

  % There are a number of weirdnesses between colortbl and longtable. I was able to iron most of them out
  % but couldn't get this to resolve without a little brute-force. --pmw
  \let\CT@cell@color\relax

} % end of AtBeginDocument{}


% ------------------------------------------------------------------------ %
% Here we define the mandatory declarations
% 
\newcommand{\SANDnum}[1]{
  \newcommand{\SANDnumVar}{#1}
  \setboolean{SANDnumProvided}{true}
}

\newcommand{\SANDauthor}[1]{
  \newcommand{\SANDauthorVar}{#1}
  \setboolean{SANDauthorProvided}{true}
}

\newcommand{\SANDprintDate}[1]{
  \newcommand{\SAND@marks@printDate}{#1}
  \setboolean{SANDprintDateProvided}{true}
}


% This one is optional
\newsavebox{\SANDrePrintDistBox}
\newsavebox{\SANDrePrintDistBoxSecond}
\newsavebox{\SANDrePrintDistBoxThird}
\newsavebox{\SANDrePrintDistBoxFourth}
\newsavebox{\SANDrePrintDistBoxFifth}
\newcommand{\SANDrePrintDateVar}{}

\newcommand{\SANDrePrintDate}[1]{
  \renewcommand{\SANDrePrintDateVar}{#1}
  \setboolean{SANDrePrintDateProvided}{true}
  \stepcounter{SANDprintingCnt}
  \ifthenelse{\value{SANDprintingCnt} = 2}   {
    \sbox{\SANDrePrintDistBoxSecond}{Second Printing, (#1):}
  }{
    \ifthenelse{\value{SANDprintingCnt} = 3}   {
      \sbox{\SANDrePrintDistBoxThird}{Third Printing, (#1):}
    }{
      \ifthenelse{\value{SANDprintingCnt} = 4}   {
        \sbox{\SANDrePrintDistBoxFourth}{Fourth Printing, (#1):}
      }{
        \ifthenelse{\value{SANDprintingCnt} = 5}   {
          \sbox{\SANDrePrintDistBoxFifth}{Fifth Printing, (#1):}
        }{
          \ClassError{SANDreport.cls}{\@backslashchar SANDreDistribution
            used more than four times!}{Do you really have more than
            five printings? If so, you need to amend SANDreport.cls}
        }
      }
    }
  }
}


% Used to need the title at the top of the first section/chapter, but that is
% not required anymore.  Make sure we start on an odd page.
\newcommand{\SANDmain}{
  \cleardoublepage
  \setboolean{SANDmainProvided}{true}
}



% ------------------------------------------------------------------------ %
% And now some optional declarations
% 
\newcommand{\SANDsupersed}[2]{
  \newsavebox{\SANDsupersedtempbox}
  \newcommand{\SANDsupersedVar}{
    Supersedes #1 \\
    Dated #2
  }
  \sbox{\SANDsupersedtempbox}{
    \begin{tabular}{c}
      Supersedes #1 \\
      dated #2
    \end{tabular}
  }
  \setboolean{SANDsupersedProvided}{true}
}


% Distribution category is no longer needed
\newcommand{\SANDdistcategory}[1]{
  \ClassWarning{SANDreport.cls}{Distribution category is no longer needed!}
}



%
% Table of Contents
%

% Change the bibliography title if we are a report 
\ifthenelse{\boolean{reportSAND}}{
  \renewcommand{\bibname}{References}
}{}

% Don't parskip below LIST OF FIGURES/TABLES headings
\setuptoc{lof}{noparskipfake}
\setuptoc{lot}{noparskipfake}

% Don't boldface section page numbers in TOC
\newcommand{\SAND@format@pgnum}[1]{\normalfont\normalcolor #1}
\DeclareTOCStyleEntry[pagenumberformat=\SAND@format@pgnum]{tocline}{section}
% Don't boldface section titles in TOC
\newcommand{\SAND@format@sectitle}[1]{\normalfont\normalcolor #1}
\DeclareTOCStyleEntry[entryformat=\SAND@format@sectitle]{tocline}{section}
% Use smaller dots in TOC entries
\renewcommand{\@dotsep}{2}

% Set fonts of headings
\ifthenelse{\boolean{reportSAND}}{
  \setkomafont{chapter}{\sffamily\large\bfseries}
  \setkomafont{section}{\sffamily\normalsize\bfseries}
  \setkomafont{subsection}{\normalsize\itshape\sffamily}
  \setkomafont{subsubsection}{\small\bfseries\sffamily}
}{
  \setkomafont{section}{\sffamily\large\bfseries\MakeUppercase}
  \setkomafont{subsection}{\sffamily\normalsize\bfseries}
  \setkomafont{subsubsection}{\normalsize\itshape\sffamily}
}


% Try to produce "tab-stopped" indentation of numbered section titles (not the numbers, which
% should still sit at the left text boundary. 2cm is arbitrarily chosen after some
% trial and error.
% 
\renewcommand{\sectionlinesformat}[4]{%
  \ifdefempty{#3}{%
    \@hangfrom{\hskip #2#3}{#4}%
  }{%
    \@hangfrom{\hbox to 2cm{#3}}{#4}%
  }
}

\ifthenelse{\boolean{reportSAND}}{
  \renewcommand{\chapterlinesformat}[3]{%
    \ifstrempty{#2}{%
      \@hangfrom{\null}{\expandafter\MakeUppercase\expandafter{#3}}%
    }{%
      \@hangfrom{\hbox to 2cm{#2}}{\expandafter\MakeUppercase\expandafter{#3}}%
    }
  }
}{}


\ifthenelse{\boolean{reportSAND}}{
  \renewcommand{\thefigure}{\thechapter-\arabic{figure}}
  \renewcommand{\thetable}{\thechapter-\arabic{table}}
  \@addtoreset{figure}{chapter}
  \@addtoreset{table}{chapter}
}{
  \renewcommand{\thefigure}{\thesection-\arabic{figure}}
  \renewcommand{\thetable}{\thesection-\arabic{table}}
  \@addtoreset{figure}{section}
  \@addtoreset{table}{section}
}
  
% Always number sections down through subsubsection
\setcounter{secnumdepth}{\subsubsectionnumdepth}


% ******************************************************************************
% Captions
% We want the label bold face, and the whole captionwidth about 4",
% and the text small
% 
\addtokomafont{caption}{\footnotesize\sffamily\bfseries}
\setkomafont{captionlabel}{\footnotesize\sffamily\bfseries}
\setcapindent{0em}
\renewcommand*{\captionformat}{\ }
\setcapwidth[c]{5in}

% \renewcommand*{\figureformat}{\figurename~\thesection-\thefigure\autodot}
% \renewcommand*{\tableformat}{\tablename~\thesection-\thetable\autodot}
\newenvironment{SANDinlinenote}{%
  \par\hspace*{.25in}\begin{minipage}{\dimexpr\textwidth-.5in}
    \rule{\dimexpr\textwidth-.25in}{.4pt}\vspace{.25\baselineskip}
    \SAND@text@justification{\bfseries\sffamily NOTE:}%
}{%
  \rule[.25\baselineskip]{\dimexpr\textwidth-.25in}{.4pt}
  \end{minipage}
}



% 
%  OUO text string and typeset box definitions
%


\newlength{\SANDUCIboxwidth}
\setlength{\SANDUCIboxwidth}{3.5in}
\newcommand{\SANDOUOcategory}{\uline{category 3: Statutory Exemption \textendash{} Export Controlled Information \textendash{} EAR.}}
\newcommand{\SANDOUOnameorg}{NAME / ORG}
\newcommand{\SANDOUOdate}{\today}
\newcommand{\SANDOUOguidance}{N/A}

\newcommand{\SANDUCIcatIII}{\uline{category 3: Statutory Exemption}}
\newcommand{\SANDUCIcatIIIEC}{\uline{category 3: Statutory Exemption \textendash{} Export Controlled Information / AEA}}
\newcommand{\SANDUCIcatIIIEAR}{\uline{category 3: Statutory Exemption \textendash{} Export Controlled Information / EAR}}
\newcommand{\SANDUCIcatIIICRADA}{\uline{category 3: Statutory Exemption}}
\newcommand{\SANDUCIcatIIIPATENT}{\uline{category 3: Statutory Exemption \textendash{} Patent Caution}}
\newcommand{\SANDUCIcatIIIITAR}{\uline{category 3: Statutory Exemption \textendash{} Export Controlled Information / ITAR}}
\newcommand{\SANDCRADAdate}[1][\today]{{#1}}
\newcommand{\SANDCRADAnumber}{999999}
\newcommand{\SANDCRADAembargo}{100 years}

\newcommand{\SANDUCNIcatIIICRADA}{\uline{category 3: Statutory Exemption}}
\newcommand{\SANDUCNIcatIIIITAR}{\uline{category 3: Statutory Exemption \textendash{} Export Controlled Information / ITAR}}
\newcommand{\SANDUCNIcatIIIEAR}{\uline{category 3: Statutory Exemption \textendash{} Export Controlled Information / EAR}}
\newcommand{\SANDUCNIcatIIIDOE}{\uline{category 3: Statutory Exemption \textendash{} Export Controlled Information / DOE}}
\newcommand{\SANDUCNIcatIIIPATENT}{\uline{category 3: Statutory Exemption \textendash{} Patent Caution}}

\newcommand{\SANDUCIcatIV}{\uline{category 4: Commercial/Proprietary}}

\newcommand{\SANDUCIcatV}{\uline{category 5: Privileged Information}}
\newcommand{\SANDUCIPIApprovals}{N/A}
\newcommand{\SANDUCIcatVI}{\uline{category 6: Personal Privacy}}
\newcommand{\SANDUCIcatVII}{\uline{category 7: Law Enforcement}}
\newcommand{\SANDUCIwhichBox}{\relax}
\newcommand{\SANDUCIwhichExtraBox}{\relax}
\newcommand{\SANDUCIwhichBonusBox}{\relax}

\newcommand{\SAND@SP@text}{SANDIA PROPRIETARY}

\newcommand{\SANDClassDCName}{Not A. DC}
\newcommand{\SANDClassDCPosition}{Derivative Classifier}
\newcommand{\SANDClassDCOrg}{DOE SNL}
\newcommand{\SANDClassDCGuide}{CG-FAKE-2, 01/01/2099, DOE OC}
\newcommand{\SANDClassDCDate}{NOT YET DC REVIEWED}
\newcommand{\SANDClassDCDeclassify}{DECLASSIFICATION DATE NEEDED}
\newcommand{\SANDClassWhichDCBox}{\relax}
\newcommand{\SANDClassWhichWarningBox}{\relax}
\newcommand{\SANDClassWhichExtraBox}{\relax}

\newcommand{\SANDSGINameTitle}{\relax}
\newcommand{\SANDSGIOrg}{\relax}
\newcommand{\SANDSGIDate}{\relax}

\newcommand{\SANDUCIcatIIIECDOEextraBox}{
  {\bfseries EXPORT CONTROLLED INFORMATION}\par
  WARNING \textendash{} This document contains technical data whose export is restricted by the Atomic Energy Act of 1954, as
  amended, 42 U.S.C. \S 2011 et seq. Violations of these export laws are subject to severe criminal penalties.
}

\newcommand{\SANDUCIcatIIIECEARextraBox}{
  {\bfseries EXPORT CONTROLLED INFORMATION}\par
  WARNING \textendash{} This document contains technical data whose export is restricted by the Export Administration Act of 1979,
  as amended, 50 U.S.C. \S 2401 et seq. Violations of these export laws are subject to severe criminal penalties.
}

\newcommand{\SANDUCIcatIIIITARextraBox}{
  {\bfseries EXPORT CONTROLLED INFORMATION}\par
  WARNING \textendash{} This document contains technical data whose export is restricted by the Arms
  Export Control Act, 22 U.S.C. 39 \S 2751 et seq. Violations of these export laws are subject to severe criminal penalties.
}

\newcommand{\SANDUCIcatIIICRADAextraBox}{%
  {\bfseries PROTECTED CRADA INFORMATION}\par
  This product contains Protected CRADA Information which was produced \SANDCRADAdate\xspace{} under CRADA No. \SANDCRADAnumber\xspace{} and is not to be further disclosed for a period of \SANDCRADAembargo\xspace{} from the date it was produced except as expressly provided for in the CRADA.
  \par\vspace{.5\baselineskip}
  Further dissemination authorized to the Department of Energy only; other requests shall be approved by the originating facility or
  higher DOE programmatic authority.\par
}

\newcommand{\SANDUCIcatIIIPATENTextraBox}{%
  {\bfseries PATENT CAUTION}\par
  These materials may reveal patentable subject matter. The information must not be divulged outside
  Sandia National Laboratories without the approval of the Legal Technology Transfer Center, Sandia
  National Laboratories. Approved external recipients must not divulge the information to others.
  \par\vspace{.5\baselineskip}
  Further dissemination authorized to U.S. Government agencies only; other requests must be approved by the originating facility or
  higher DOE programmatic authority.
  \par\vspace{.5\baselineskip}
  No distribution limitation for Patent Interest.\par
}


\newcommand{\SANDUCIcatIVextraBox}{%
  \par\vspace{\baselineskip}
  \parbox{2.75in}{\bfseries \SAND@marks@thirdPartyProprietaryName\xspace{} Proprietary}\par
  This technical data contains \SAND@marks@thirdPartyProprietaryName\xspace{} Proprietary Information furnished under
  contract or agreement between Sandia National Laboratories and \SAND@marks@thirdPartyProprietaryName\xspace{} for the
  controlled release of the information. Disclosure outside the Government is not authorized without
  prior approval of the originator, or in accordance with provisions of 48 CFR 952.227 and 5
  U.S.C. 552.\par\vspace{.5\baselineskip}
  Further dissemination authorized to U.S. Government agencies only; other requests must be approved by
  the originating facility or higher DOE programmatic authority.\par
}

\newcommand{\SANDUCIcatVPIextraBox}{%
  \par\vspace{\baselineskip}
  Any further distribution by any holder of this product or data therein to third parties representing foreign interests, foreign
  governments, foreign companies, and foreign subsidiaries or foreign divisions of U.S. companies shall be approved by the
  \SANDUCIPIApprovals\xspace{}, U.S. Department of Energy. Further, foreign party release may require DOE approval pursuant to 10 CFR 810,
  and/or may be subject to Section 127 of the Atomic Energy Act.
  \par\vspace{.5\baselineskip}
  Further dissemination only as authorized by the originating facility or higher DOE programmatic authority; requester must possess appropriate security clearance, need-to-know, and facility approval for receipt and storage of classified documents by the DOE Office of Security Affairs.
}

\newcommand{\SANDUCNICRADAbottomBox}{
  {\bfseries PROTECTED CRADA INFORMATION}\par
  This product contains Protected CRADA Information which was produced \SANDCRADAdate\xspace{} under CRADA No. \SANDCRADAnumber\xspace{} and is not to be further disclosed for a period of \SANDCRADAembargo\xspace{} from the date it was produced except as expressly provided for in the CRADA.
  \par\vspace{.5\baselineskip}
  Further dissemination authorized to the Department of Energy only; other requests shall be approved by the originating facility or
  higher DOE programmatic authority.\par
}

\newcommand{\SANDUCNIPATENTbottomBox}{%
  {\bfseries PATENT CAUTION}\par
  These materials may reveal patentable subject matter. The information must not be divulged outside
  Sandia National Laboratories without the approval of the Legal Technology Transfer Center, Sandia
  National Laboratories. Approved external recipients must not divulge the information to others.
  \par\vspace{.5\baselineskip}
  Further dissemination authorized to U.S. Government agencies only; other requests must be approved by the originating facility or
  higher DOE programmatic authority.
  \par\vspace{.5\baselineskip}
  No distribution limitation for Patent Interest.\par
}

\newcommand{\SANDUCNIITARbottomBox}{
  \parbox{\SAND@bottombox@width}{\bfseries\centering EXPORT CONTROLLED INFORMATION}
  \par\vspace{.5\baselineskip}
  WARNING \textendash{} This document contains technical data whose export is restricted by the Arms Export Control Act, 22 U.S.C. 39 \S 2751 et seq. Violations of these export laws are subject to severe criminal penalties.
}

\newcommand{\SANDUCNIEARbottomBox}{
  \parbox{\SAND@bottombox@width}{\bfseries\centering EXPORT CONTROLLED INFORMATION}
  \par\vspace{.5\baselineskip}
  WARNING \textendash{} This document contains technical data whose export is restricted by the Export Administration Act of 1979,
  as amended, 50 U.S.C. App. 2401 et seq. Violations of these export laws are subject to severe criminal penalties.
}

\newcommand{\SANDUCNIDOEbottomBox}{
  \parbox{\SAND@bottombox@width}{\bfseries\centering EXPORT CONTROLLED INFORMATION}
  \par\vspace{.5\baselineskip}
  WARNING \textendash{} This document contains technical data whose export is restricted by the Atomic Energy Act of 1954,
  as amended, 42 U.S.C. \S 2401 et seq. Violations of these export laws are subject to severe criminal penalties.
}

\newcommand{\SANDClassWarningNSI}{
    \parbox{5in}{\bfseries DERIVATIVE DECLASSIFIER REVIEW REQUIRED PRIOR TO DECLASSIFICATION}
}

\newcommand{\SANDClassWarningFRD}{
    \parbox{3in}{\bfseries\centering FORMERLY RESTRICTED DATA}
    \par\vspace{.5\baselineskip}
    Unauthorized disclosure subject to Administrative and Criminal Sanctions. Handle as Restricted Data in Foreign Dissemination, Section 144.b, Atomic Energy Act, 1954.
}

\newcommand{\SANDClassWarningRD}{
    \parbox{3in}{\bfseries\centering RESTRICTED DATA}
    \par\vspace{.5\baselineskip}
    This document contains Restricted Data as defined in the Atomic Energy Act of 1954. Unauthorized disclosure subject to Administrative and Criminal Sanctions.
}

\newcommand{\SANDSGIWarning}{
  \parbox{3in}{\bfseries\centering SAFEGUARDS INFORMATION}
  \par\vspace{.5\baselineskip}
  {\bfseries Unauthorized disclosure will be subject to CIVIL and CRIMINAL sanctions.}
  \par\vspace{.5\baselineskip}
  Safeguards Information determination made by:
  \par\vspace{.5\baselineskip}
  Name/Title: \SANDSGINameTitle
  \par\vspace{.5\baselineskip}
  Organization: \SANDSGIOrg
  \par\vspace{.5\baselineskip}
  Date: \SANDSGIDate
  \par\vspace{.5\baselineskip}
}

\newcommand{\SANDUNNPIWarning}{
  \parbox{3in}{\bfseries\centering UNCLASSIFIED NAVAL NUCLEAR PROPULSION INFORMATION}
  \par\vspace{.5\baselineskip}
  No Foreign Dissemination (NOFORN). This document is subject to special export controls and each
  transmittal to foreign governments or foreign nationals must be made only with the prior approval of
  Naval Sea Systems Command.
  }

% 
% Define some boxes to use on the cover pages
% 
\newcommand{\SANDUCIbox}[2][1.5in]{%
  \framebox{%
    \vspace{.1in plus 1fill}
    \begin{minipage}[t][#1][c]{#2}
      \scriptsize\sffamily
      \parbox{#2}{\bfseries\centering OFFICIAL USE ONLY}
      \vspace{.05in}\par\raggedright
      May be exempt from public release under the Freedom of Information Act (5 U.S.C. 552), and \SANDOUOcategory\xspace{}.
      \par\vspace{.5\baselineskip}
      Department of Energy review required before public release.
      \par\vspace{.5\baselineskip}
      Name/Org: \SANDOUOnameorg \hfill Date: \SANDOUOdate
      \par\vspace{.5\baselineskip}
      Guidance (if applicable): \SANDOUOguidance
    \end{minipage}%
    \vspace{.1in plus 1fill}
  }%
}

\newcommand{\makeUCIextrabox}[2]{%
  \framebox{
    \vspace{.05in plus 1fill}
    \begin{minipage}[t][1.5in][c]{#1}
      \scriptsize\sffamily\raggedright #2%
    \end{minipage}%
    \vspace{.05in plus 1fill}
  }
}

\newlength{\SAND@ucnibox@width}
\setlength{\SAND@ucnibox@width}{3in}
\newcommand{\SANDUCNIbox}{%
  \framebox{%
    \vspace{.1in plus 1fill}
    \begin{minipage}[t][1.5in][c]{\SAND@ucnibox@width}
      \scriptsize\sffamily
      \parbox{\SAND@ucnibox@width}{\bfseries\centering UNCLASSIFIED CONTROLLED NUCLEAR INFORMATION \newline NOT FOR PUBLIC DISSEMINATION}\par
      \vspace{.1in}\raggedright
      Unauthorized dissemination subject to civil and criminal sanctions under Section 148 of the Atomic Energy Act of 1954, as amended
      (42 U.S.C. 2168).\par\vspace{.5\baselineskip}%
      Reviewing Official/Org: \SANDOUOnameorg \par\vspace{.5\baselineskip}%
      Date: \SANDOUOdate \par\vspace{.5\baselineskip}%
      Guidance (if applicable): \SANDOUOguidance %
    \end{minipage}%
    \vspace{.1in plus 1fill}
  }%
}

\newlength{\SAND@bottombox@width}
\setlength{\SAND@bottombox@width}{6.475in}
\newcommand{\makeUCNIbottombox}[1]{%
  \framebox{
    \vspace{.2in plus 1fill}
    \begin{minipage}[t][][c]{\SAND@bottombox@width}%
      \scriptsize\sffamily\raggedright #1 %
    \end{minipage}%
    \vspace{.2in plus 1fill}
  }
}

\newcommand{\makeOpenBottomBox}[1]{%
  \vspace{.2in plus 1fill}
  \begin{minipage}[t][][c]{\SAND@bottombox@width}%
    \scriptsize\sffamily\raggedright #1 %
  \end{minipage}%
  \vspace{.2in plus 1fill}
}

\newcommand{\makeClassDCBox}{%
    \framebox{%
      \begin{minipage}[t][][c]{3in}
        \scriptsize\sffamily
        Classified by: \SANDClassDCName, \SANDClassDCPosition, \SANDClassDCOrg \par
        Derived from: \SANDClassDCGuide \par
        Date classified: \SANDClassDCDate
        \iftoggle{SAND@CLASS@NSI}{\par Declassify on: \SANDClassDCDeclassify}{}
      \end{minipage}%
    }
%    \iftoggle{SAND@CLASS@NSI}{
%      DERIVATIVE DECLASSIFIER REVIEW REQUIRED PRIOR TO DECLASSIFICATION
%    }{}
}

\newcommand{\makeClassWarningBox}[1]{%
    \framebox{
        \begin{minipage}[t][][c]{3in}
            \scriptsize\sffamily\raggedright #1%
        \end{minipage}%
    }
}

\newcommand{\makeClassWarningBoxNSI}[1]{%
        \begin{minipage}[t][][c]{3in}
            \scriptsize\sffamily\raggedright #1%
        \end{minipage}%
}

\newcommand{\makeClassExtraBox}[1]{%
    \begin{minipage}[t][][c]{3in}
      \scriptsize\sffamily\raggedright #1%
    \end{minipage}%
}

% Determine which "admonishment" boxes to apply to the cover page
% For reference, here are the relevant toggles
% \newtoggle{SAND@OUOtype@UCI}
% \newtoggle{SAND@OUOtype@UCNI}

% \newtoggle{SAND@FOIA@statutory}
% \newtoggle{SAND@FOIA@proprietary}
% \newtoggle{SAND@FOIA@privileged}
% \newtoggle{SAND@FOIA@pii}
% \newtoggle{SAND@FOIA@lawenforcement}

% \newtoggle{SAND@FOIA@ECDOE}
% \newtoggle{SAND@FOIA@CRADA}
% \newtoggle{SAND@FOIA@ECEAR}
% \newtoggle{SAND@FOIA@ECITAR}
% \newtoggle{SAND@FOIA@PATENT}

% This is not the most efficient "programming" practice here, since
% we'll always do a few comparisons that could be short-circuited (all of the
% options are mutually exclusive). I'm doing it this way for
% readability since LaTeX makes an if-then cascade kind of ugly, and I'd
% rather optimize for maintainability vs. compile time.

% These category strings are the same for UCI and UCNI, so do them separately.

% User API for specifying a custom FOIA string. Can be used to give multiple FOIA categories (until the
% class is updated to allow that through the document options). If they specify OUO=custom, they need to
% have set this or they'll get nothing typeset.
\newcommand{\SANDFOIAcustomCategory}[2]{
  \newcommand{\SAND@marks@FOIAcustom}{#1}
  \newcommand{\SAND@marks@headfoot}{#2}
}


\iftoggle{SAND@OUOtype@UCI}{

  % Begin by assuming the generic FOIA 3 exemption, and reset it if we were given something different
  \renewcommand{\SANDOUOcategory}{\SANDUCIcatIII}
  \renewcommand{\SAND@marks@releaseType}{Official Use Only}
  \renewcommand{\SAND@marks@headfoot}{OFFICIAL USE ONLY}
  \renewcommand{\SANDUCIwhichBox}{\SANDUCIbox{3in}}  

  \iftoggle{SAND@FOIA@custom}{
    \renewcommand{\SANDOUOcategory}{\SAND@marks@FOIAcustom}
    \renewcommand{\SAND@marks@releaseType}{Official Use Only}
  }{}
  \iftoggle{SAND@FOIA@proprietary}{
    \renewcommand{\SANDOUOcategory}{\SANDUCIcatIV}
    \renewcommand{\SAND@marks@releaseType}{Official Use Only / Sandia Proprietary}
    \renewcommand{\SAND@marks@headfoot}{OFFICIAL USE ONLY / SANDIA PROPRIETARY}
  }{}
  \iftoggle{SAND@FOIA@privileged}{
    \renewcommand{\SANDOUOcategory}{\SANDUCIcatV}
    \renewcommand{\SAND@marks@releaseType}{Official Use Only / Applied Technology}
    \renewcommand{\SANDUCIwhichBonusBox}{\makeOpenBottomBox{\SANDUCIcatVPIextraBox}}
    \renewcommand{\SAND@marks@headfoot}{OFFICIAL USE ONLY / APPLIED TECHNOLOGY}
  }{}
  \iftoggle{SAND@FOIA@pii}{
    \renewcommand{\SANDOUOcategory}{\SANDUCIcatVI}
    \renewcommand{\SAND@marks@releaseType}{Official Use Only / Personally Identifiable Information}
    \renewcommand{\SAND@marks@headfoot}{OFFICIAL USE ONLY / PERSONALLY IDENTIFIABLE INFORMATION}
  }{}
  \iftoggle{SAND@FOIA@lawenforcement}{
    \renewcommand{\SANDOUOcategory}{\SANDUCIcatVII}
    \renewcommand{\SAND@marks@releaseType}{Official Use Only / Law Enforcement}
    \renewcommand{\SAND@marks@headfoot}{OFFICIAL USE ONLY / LAW ENFORCEMENT}
  }{}%

  % three possibilities for an extra box in UCI 3
  \iftoggle{SAND@FOIA@ECDOE}{
    \renewcommand{\SANDUCIwhichBox}{\SANDUCIbox{3in}}
    \renewcommand{\SANDUCIwhichExtraBox}{\makeUCIextrabox{2.75in}{\SANDUCIcatIIIECDOEextraBox}}
    \renewcommand{\SANDOUOcategory}{\SANDUCIcatIIIEC}
    \renewcommand{\SAND@marks@releaseType}{Official Use Only / Export Controlled Information}
    \renewcommand{\SAND@marks@headfoot}{OFFICIAL USE ONLY / EXPORT CONTROLLED INFORMATION}
  }{}
  \iftoggle{SAND@FOIA@CRADA}{
    \renewcommand{\SANDUCIwhichBox}{\SANDUCIbox{2.75in}}
    \renewcommand{\SANDUCIwhichExtraBox}{\makeUCIextrabox{3in}{\SANDUCIcatIIICRADAextraBox}}
    \renewcommand{\SANDOUOcategory}{\SANDUCIcatIIICRADA}
    \renewcommand{\SAND@marks@releaseType}{Official Use Only / CRADA}
  }{}
  \iftoggle{SAND@FOIA@PATENT}{
    \renewcommand{\SANDUCIwhichBox}{\SANDUCIbox{3in}}
    \renewcommand{\SANDUCIwhichExtraBox}{\makeUCIextrabox{3.25in}{\SANDUCIcatIIIPATENTextraBox}}
    \renewcommand{\SANDOUOcategory}{\SANDUCIcatIIIPATENT}
    \renewcommand{\SAND@marks@releaseType}{Official Use Only / Patent Caution}
  }{}
  \iftoggle{SAND@FOIA@ECEAR}{
    \renewcommand{\SANDUCIwhichBox}{\SANDUCIbox{3in}}
    \renewcommand{\SANDUCIwhichExtraBox}{\makeUCIextrabox{2.75in}{\SANDUCIcatIIIECEARextraBox}}
    \renewcommand{\SANDOUOcategory}{\SANDUCIcatIIIEAR}
    \renewcommand{\SAND@marks@releaseType}{Official Use Only / Export Controlled Information}
    \renewcommand{\SAND@marks@headfoot}{OFFICIAL USE ONLY / EXPORT CONTROLLED INFORMATION}
  }{}
  \iftoggle{SAND@FOIA@ECITAR}{
    \renewcommand{\SANDUCIwhichBox}{\SANDUCIbox{3in}}
    \renewcommand{\SANDUCIwhichExtraBox}{\makeUCIextrabox{2.75in}{\SANDUCIcatIIIITARextraBox}}
    \renewcommand{\SANDOUOcategory}{\SANDUCIcatIIIITAR}
    \renewcommand{\SAND@marks@releaseType}{Official Use Only / Export Controlled Information}
    \renewcommand{\SAND@marks@headfoot}{OFFICIAL USE ONLY / EXPORT CONTROLLED INFORMATION}
  }{}

}{} % UCI

\iftoggle{SAND@OUOtype@UCNI}{
  \renewcommand{\SANDUCIwhichBox}{\SANDUCIbox{3in}}
  \renewcommand{\SANDUCIwhichExtraBox}{\SANDUCNIbox}
  \renewcommand{\SAND@marks@releaseType}{Official Use Only / Unclassified Controlled Nuclear Information}
  \renewcommand{\SAND@marks@headfoot}{UNCLASSIFIED CONTROLLED NUCLEAR INFORMATION}
  % Only two extra cases for UCNI:
  % - only need a bonus box with FOIA exemption 3
  % - FOIA 4,5,6,7 have no bonus box so we're done
  \iftoggle{SAND@FOIA@statutory}{
    \renewcommand{\SANDOUOcategory}{\SANDUCIcatIII}
    \iftoggle{SAND@FOIA@ECDOE}{
      \renewcommand{\SANDUCIwhichBonusBox}{\makeUCNIbottombox{\SANDUCNIDOEbottomBox}}
      \renewcommand{\SANDOUOcategory}{\SANDUCNIcatIIIDOE}
      \renewcommand{\SAND@marks@releaseType}{Official Use Only / UCNI / Export Controlled Information}
      \renewcommand{\SAND@marks@headfoot}{OFFICIAL USE ONLY / UCNI / EXPORT CONTROLLED INFORMATION}
    }{}
    \iftoggle{SAND@FOIA@CRADA}{
      \renewcommand{\SANDUCIwhichBonusBox}{\makeUCNIbottombox{\SANDUCNICRADAbottomBox}}
      \renewcommand{\SANDOUOcategory}{\SANDUCNIcatIIICRADA}
      \renewcommand{\SAND@marks@releaseType}{Official Use Only / UCNI / CRADA}
      \renewcommand{\SAND@marks@headfoot}{OFFICIAL USE ONLY / UCNI / CRADA}
    }{}
    \iftoggle{SAND@FOIA@PATENT}{
      \renewcommand{\SANDUCIwhichBonusBox}{\makeUCNIbottombox{\SANDUCNIPATENTbottomBox}}
      \renewcommand{\SANDOUOcategory}{\SANDUCNIcatIIIPATENT}
      \renewcommand{\SAND@marks@releaseType}{Official Use Only / UCNI / Patent Caution}
      \renewcommand{\SAND@marks@headfoot}{OFFICIAL USE ONLY / UCNI/ Patent Caution}
    }{}
    \iftoggle{SAND@FOIA@ECEAR}{
      \renewcommand{\SANDUCIwhichBonusBox}{\makeUCNIbottombox{\SANDUCNIEARbottomBox}}
      \renewcommand{\SANDOUOcategory}{\SANDUCNIcatIIIEAR}
      \renewcommand{\SAND@marks@releaseType}{Official Use Only / UCNI / Export Controlled Information}
      \renewcommand{\SAND@marks@headfoot}{OFFICIAL USE ONLY / UCNI / EXPORT CONTROLLED INFORMATION}
    }{}
    \iftoggle{SAND@FOIA@ECITAR}{
      \renewcommand{\SANDUCIwhichBonusBox}{\makeUCNIbottombox{\SANDUCNIITARbottomBox}}
      \renewcommand{\SANDOUOcategory}{\SANDUCNIcatIIIITAR}
      \renewcommand{\SAND@marks@releaseType}{Official Use Only / UCNI / Export Controlled Information}
      \renewcommand{\SAND@marks@headfoot}{OFFICIAL USE ONLY / UCNI / EXPORT CONTROLLED INFORMATION}
    }{}%
  }{} % statutory
} % UCNI

% Add SP marks if needed
\iftoggle{SAND@SP}{
  \ifx\SAND@marks@headfoot\empty
  \def\SAND@marks@headfoot{\SAND@SP@text}
  \else
  \g@addto@macro\SAND@marks@headfoot{ / \SAND@SP@text}
  \fi
}{}

\iftoggle{SAND@CLASS@CONFIDENTIAL}{
	\renewcommand{\SAND@marks@releaseType}{Confidential}
	\renewcommand{\SAND@marks@headfoot}{CONFIDENTIAL}
}{}
\iftoggle{SAND@CLASS@SECRET}{
	\renewcommand{\SAND@marks@releaseType}{Secret}
	\renewcommand{\SAND@marks@headfoot}{SECRET}
}{}
\iftoggle{SAND@CLASS@TOPSECRET}{
	\renewcommand{\SAND@marks@releaseType}{Top Secret}
	\renewcommand{\SAND@marks@headfoot}{TOP SECRET}
}{}

\iftoggle{SAND@CLASS@FRD}{
	\let\classleveltype\SAND@marks@releaseType
	\renewcommand{\SAND@marks@releaseType}{\classleveltype{} Formerly Restricted Data}
	\let\classlevelhead\SAND@marks@headfoot
	\renewcommand{\SAND@marks@headfoot}{\classlevelhead{} FORMERLY RESTRICTED DATA}
    \renewcommand{\SANDClassWhichWarningBox}{\makeClassWarningBox{\SANDClassWarningFRD}}
}{}
\iftoggle{SAND@CLASS@RD}{
	\let\classleveltype\SAND@marks@releaseType
	\renewcommand{\SAND@marks@releaseType}{\classleveltype{} Restricted Data}
	\let\classlevelhead\SAND@marks@headfoot
	\renewcommand{\SAND@marks@headfoot}{\classlevelhead{} RESTRICTED DATA}
    \renewcommand{\SANDClassWhichWarningBox}{\makeClassWarningBox{\SANDClassWarningRD}}
}{}

\iftoggle{SAND@CLASS}{
  \renewcommand{\SANDClassWhichDCBox}{\makeClassDCBox}
  \iftoggle{SAND@CLASS@NSI}{
    \renewcommand{\SANDClassWhichExtraBox}{\makeClassExtraBox{\SANDClassWarningNSI}
    }{}
}{}

}{}

\iftoggle{SAND@CLASS@NF}{
	\let\classtype\SAND@marks@releaseType
	\renewcommand{\SAND@marks@releaseType}{\classtype{} // NOFORN}
	\let\classhead\SAND@marks@headfoot
	\renewcommand{\SAND@marks@headfoot}{\classhead{} // NOFORN}
    \renewcommand{\SANDClassWhichExtraBox}{\makeClassExtraBox{No Foreign Dissemination}}
}{}
\iftoggle{SAND@CLASS@S15}{
	\let\classtype\SAND@marks@releaseType
	\renewcommand{\SAND@marks@releaseType}{\classtype{} // Sigma 15}
	\let\classhead\SAND@marks@headfoot
	\renewcommand{\SAND@marks@headfoot}{\classhead{} // SIGMA 15}
    \renewcommand{\SANDClassWhichExtraBox}{\makeClassExtraBox{SIGMA 15}}
}{}

\newcommand{\SANDThirdPartyProprietary}[1]{
  \newcommand{\SAND@marks@thirdPartyProprietaryName}{#1}
  \renewcommand{\SAND@marks@releaseType}{Official Use Only/Third Party Proprietary Information}
  \renewcommand{\SANDUCIwhichBonusBox}{\makeOpenBottomBox{\SANDUCIcatIVextraBox}}
  \renewcommand{\SAND@marks@headfoot}{OFFICIAL USE ONLY/THIRD PARTY PROPRIETARY}    
}


%%
%%
%% end of marking definitions
%%
%%%%%%%%%%%%%%

%%%%%%%%%%%%%%
%%
%% Begin definition of cover page layers
%%
%%
\iftoggle{SAND@OUO}{
  \newcommand{\SANDcoverBackground}{SANDOUO-cover-bg}
  \newcommand{\SANDbackcoverBackground}{SANDOUO-back-bg}
}{   
  \iftoggle{SAND@CLASS}{  
      \newcommand{\SANDcoverBackground}{SANDclass-cover-bg}
      \newcommand{\SANDbackcoverBackground}{SANDclass-back-bg}
  }{  
      \newcommand{\SANDcoverBackground}{SANDcover-bg}
      \newcommand{\SANDbackcoverBackground}{SANDbackcover-bg}
  }   
}

% background image layer to apply to cover page
\DeclareNewLayer[background,page, mode=picture, contents={%
  \putLL{\includegraphics{\SANDcoverBackground}}}]{SANDcover-bg}


% cover content layer with SANDA REPORT, SAND number, printed date 
\DeclareNewLayer[foreground,mode=text,area={1in}{1.5in}{4in}{1.5in},contents={%
  % 
  \color{white}\sffamily
  {\huge\bfseries SANDIA REPORT}\\\SANDnumVar\\\ifdefempty{\SAND@marks@releaseType}{}{\SAND@marks@releaseType\\}Printed \SAND@marks@printDate}]{SANDcover-fg}
% 

\DeclareNewLayer[foreground,mode=text,area={6in}{9.75in}{2in}{1in},contents={%
  \raggedright\sffamily\scriptsize Prepared by \newline Sandia National Laboratories \newline
  Albuquerque, New Mexico 87185 \newline Livermore, California 94550\vfill}
]{SANDcover-prepared}


% 
% Renew the maketitle macro to generate the title page and other upfront material
% 
\renewcommand{\maketitle}{%
  % 
  \DeclareNewLayer[foreground,mode=text,align=lt,area={1in}{3in}{6.5in}{4in},contents={%
    \parbox[t]{6.5in}{\raggedright\linespread{2}\fontfamily{phv}\selectfont\sffamily {\bfseries\huge \@title}}%
    \vspace{0.2in}
    \parbox[b]{6.5in}{\fontfamily{phv}\selectfont\sffamily {\large \SANDauthorVar }}%
  }]{SANDcover-title}
  \DeclarePageStyleByLayers{SANDcover}{SANDcover-bg,SANDcover-fg,SANDcover-prepared,SANDcover-title}
  % 
  \iftoggle{SAND@OUOorCLASS}{
    % Add a layer containing each not-relaxed OUO admonishment 
    \ifcsvoid{SANDUCIwhichBox}{}{
      \DeclareNewLayer[foreground,mode=text,hoffset=1in,voffset=6.5in,width=\textwidth,align=lt,contents={\SANDUCIwhichBox}]{SAND@layer@uciboxes}
      \AddLayersToPageStyle{SANDcover}{SAND@layer@uciboxes}
    }%
    \ifcsvoid{SANDUCIwhichExtraBox}{}{
      \ModifyLayer[addcontents={\hspace{.3in}\SANDUCIwhichExtraBox}]{SAND@layer@uciboxes}
    }%
    \ifcsvoid{SANDUCIwhichBonusBox}{}{
      \ModifyLayer[addcontents={\\\vspace{.1in}\SANDUCIwhichBonusBox}]{SAND@layer@uciboxes}
    }%
    \ifcsvoid{SANDClassWhichWarningBox}{}{
        \ModifyLayer[addcontents={\SANDClassWhichWarningBox}]{SAND@layer@uciboxes}
    }%
    \ifcsvoid{SANDClassWhichDCBox}{}{
        \ModifyLayer[addcontents={\SANDClassWhichDCBox}]{SAND@layer@uciboxes}
    }%
    \ifcsvoid{SANDClassWhichExtraBox}{}{
        \ModifyLayer[addcontents={\\\vspace{.2in}\SANDClassWhichExtraBox}]{SAND@layer@uciboxes}
    }%
    % 
    \DeclareNewLayer[foreground,mode=text,head,contents={\parbox{\textwidth}{\sffamily\bfseries\centering\color{white} \SAND@marks@headfoot}}]{SANDcover-header}%
    \DeclareNewLayer[foreground,mode=text,foot,contents={\parbox{\textwidth}{\sffamily\bfseries\centering\color{white} \SAND@marks@headfoot}}]{SANDcover-footer}%
    \AddLayersToPageStyle{SANDcover}{SANDcover-header,SANDcover-footer}%
  }%

  \thispagestyle{SANDcover}%
  \null % force page to render with no explicit content
  
  \clearpage

  % First page after cover, with DOE disclaimer notice, DOE seal, contact addresses on UUR reports

  % Header/footer is the same for the rest of the document (until the back cover which is dealt with
  % separately in AtEndDocument). Set the plain heading style and clear the pre-set page number location.
  \pagestyle{plain.scrheadings}
  \clearplainofpairofpagestyles
  
  % For clarity a separate if-then to deal with headings
  \iftoggle{SAND@OUOorCLASS}{ % we have to add the document classification type tag in the header/footer
    \setlength{\footskip}{.5\footskip}      
    \setkomafont{pageheadfoot}{\sffamily\bfseries}
    \chead*{\SAND@marks@headfoot}
    \cfoot*{\pagemark\\\SAND@marks@headfoot}
  }{
    % Since we are a two-sided document, we have to reset the page numbers to the center even if not
    % an OUO/CLASS report
    \setlength{\footskip}{.5\footskip}
    \setkomafont{pageheadfoot}{\sffamily\bfseries}
    \cfoot*{\pagemark}
  }

  \footnotesize\begin{flushleft}
  Issued by Sandia National Laboratories, operated for the United States Department of Energy by
  National Technology \& Engineering Solutions of Sandia, LLC.

  {\bfseries NOTICE:} This report was prepared as an account of work sponsored by an agency of the
  United States Government. Neither the United States Government, nor any agency thereof, nor any of
  their employees, nor any of their contractors, subcontractors, or their employees, make any
  warranty, express or implied, or assume any legal liability or responsibility for the accuracy,
  completeness, or usefulness of any information, apparatus, product, or process disclosed, or
  represent that its use would not infringe privately owned rights. Reference herein to any specific
  commercial product, process, or service by trade name, trademark, manufacturer, or otherwise, does
  not necessarily constitute or imply its endorsement, recommendation, or favoring by the United
  States Government, any agency thereof, or any of their contractors or subcontractors. The views and
  opinions expressed herein do not necessarily state or reflect those of the United States
  Government, any agency thereof, or any of their contractors.

  Printed in the United States of America. This report has been reproduced directly from the best
  available copy.

  \iftoggle{SAND@OUOorCLASS}{}{
    Available to DOE and DOE contractors from
    
    \hspace*{.2in} % Don't separate this from the line below
    \begin{tabular}{ll}
        \multicolumn{2}{l}{U.S. Department of Energy} \\
        \multicolumn{2}{l}{Office of Scientific and Technical Information} \\
        \multicolumn{2}{l}{P.O. Box 62} \\
        \multicolumn{2}{l}{Oak Ridge, TN 37831} \\
        \\
        Telephone: & (865) 576-8401 \\
        Facsimile: & (865) 576-5728 \\
        E-Mail: & reports@osti.gov \\
        Online ordering: & http://www.osti.gov/scitech \\
    \end{tabular}

    Available to the public from

    \hspace*{.2in} % same!
    \begin{tabular}{ll}
      \multicolumn{2}{l}{U.S. Department of Commerce} \\
      \multicolumn{2}{l}{National Technical Information Service} \\
      \multicolumn{2}{l}{5301 Shawnee Road} \\
      \multicolumn{2}{l}{Alexandria, VA 22312} \\
                                 & \\
      Telephone: & (800) 553-6847 \\
      Facsimile: &  (703) 605-6900 \\
      E-Mail: & orders@ntis.gov \\
      Online order: & https://classic.ntis.gov/help/order-methods \\
    \end{tabular}
  }
  
  \includegraphics[height=1in]{DOEbwlogo}\includegraphics[height=0.45in]{NNSA_logo}
  \end{flushleft}
  \clearpage
  %
  % Generate the inner title page if the user asked for one
  %
  \if@SAND@innertitle
  %
  \SAND@typeout{Generating inner title page}
  \begin{center}%
    \let\footnote\thanks
    \vspace*{2\baselineskip}
    {\LARGE\@title\par}%
    \vspace{2\baselineskip}
    \large\begin{tabular}[t]{c}%   
      \@author
    \end{tabular}\par%
    \vfill
    \SANDnumVar\par
  \end{center}%
  \clearpage
 %
  \fi
  \normalsize
  \SAND@text@justification
}

\renewenvironment{abstract}{%
  \setlength{\parskip}{1ex}
  \setlength{\parindent}{0pt}
  {\bfseries\sffamily\MakeUppercase{\abstractname}}\par
}{}

% 
% Set up the appendix
% We clearpage so the first appendix always begins on a new page. Any following appendix sections
% will appear in-line with no page break - if the user wants that look, then they can insert
% clearpages manually between their sections.
% 
\renewcommand*\appendix{
  \clearpage
  \setcounter{section}{0}
  \setcounter{subsection}{0}
  \setcounter{subsubsection}{0}
  \setcounter{figure}{0}
  \setcounter{table}{0}

  \ifthenelse{\boolean{reportSAND}}{
    \setcounter{chapter}{0}
    % in report mode, insert a part TOC heading for the appendices - in
    % article mode, they're just orphan-ish sections. We don't want it
    % numbered (hence \chapter*), but we do want a TOC entry, which we have to
    % do ourselves
    \addcontentsline{toc}{chapter}{Appendices}
  }{}
    
  \ifthenelse{\boolean{reportSAND}}{
    \renewcommand*{\chapterformat}{APPENDIX~\thechapter\autodot\enskip}
    \renewcommand*{\chaptermarkformat}{APPENDIX~\thechapter\autodot\enskip}
    \gdef\@chapapp{\appendixname}
    \gdef\thechapter{\@Alph\c@chapter}
  }{
    \renewcommand*{\sectionformat}{APPENDIX~\thesection\autodot\enskip}
    \renewcommand*{\sectionmarkformat}{APPENDIX~\thesection\autodot\enskip}
    \gdef\@sectionapp{\appendixname}
    \gdef\thesection{\@Alph\c@section}
  }
  
  % Reset these from the main document definition since the appendix sections don't need
  % that "tab stopped" heading layout. This would trash this behavior for any following sections,
  % but the only thing after this is the Distribution page, which we generate ourselves and has no
  % subsections.
  \renewcommand*{\sectionlinesformat}[4]{%
    \@hangfrom{\hskip ##2##3}{##4}%
  }
  \ifthenelse{\boolean{reportSAND}}{
    \renewcommand*{\chapterlinesformat}[3]{%
      \@hangfrom{##2}{##3}%
    }
  }{}

  % Some evil redefinition of \addcontentsline just so we can get "Appendix" in front of the
  % appendix TOC entries. Stolen from the appendix package (which does this for the user if
  % they're using a class with a \chapter command; scrartcl, based on the legacy article class,
  % doesn't have one).
  % 
  % Like the above, this only suffices here because the appendices are the last things in the SAND
  % report that will be in the TOC. You'd need to restore the original \addcontentsline in
  % case something else needs to be in the TOC that follows the appendices.
  % 
  \let\oldacl@pp=\addcontentsline
  \def\addcontentsline##1##2##3{%
    \def\@pptempa{##1}\def\@pptempb{toc}%
    \ifx\@pptempa\@pptempb
    % if we're doing the TOC and not another list
    \def\@pptempa{##2}\def\@pptempb{section}%
    % }
    %   then if we're adding a section
    \ifx\@pptempa\@pptempb
    \ifthenelse{\boolean{reportSAND}}{
      \oldacl@pp{##1}{##2}{##3}
    }{
      % Add a custom section entry with "Appendix" prepended to the number
      \oldacl@pp{##1}{##2}{Appendix\space ##3}
    }
    \else
    % Not doing a section, just call the original addcontentsline
    \oldacl@pp{##1}{##2}{##3}%
    \fi
    \else
    % Not doing the TOC, just call the original addcontentsline
    \oldacl@pp{##1}{##2}{##3}%
    \fi}%

  \setcounter{secnumdepth}{2}
}

% ------------------------------------------------------------------------ %
% Distribution page at the end
% We have to create our own description environment, so we don't
% disturb the original one.

\newenvironment{SANDdistribution}[1][nothing]{
  \setboolean{SANDdistributionProvided}{true}

  \newtoggle{distCRADA}
  \newtoggle{distPatent}
  \newtoggle{distLDRD}
  \newtoggle{distClassified}
  \newcommand{\SANDdistCRADA}{
    \toggletrue{distCRADA}
  }

  \newcommand{\SANDdistPatent}{
    \toggletrue{distPatent}
  }

  \newcommand{\SANDdistLDRD}{
    \toggletrue{distLDRD}
  }

  \newcommand{\SANDdistClassified}{
    \toggletrue{distClassified}
    \SANDdistInternalM{2}{M2497}{Central Technical Files}{8944}
  }



  % Option NM given
  \ifthenelse{\equal{#1}{NM}}   {
    \SAND@typeout{Option "NM": Using the SNL NM default distribution list.}
    \setboolean{distNM}{true}
  }{
  }

  % Option CA given
  \ifthenelse{\equal{#1}{CA}}   {
    \SAND@typeout{Option "CA": Using the SNL CA default distribution list.}
    \setboolean{distCA}{true}
  }{
  }

  %
  %  Where in previous SANDreport style it was OK to just typeset the distribution items as they were
  %  encountered, now a table display is required. Since we don't know how many items we'll have, we need
  %  to collect each distribution item and do all the typesetting when the SANDdistribution environment
  %  closes. We can use token registers to do this.
  %
  \newtoks\IHtoks\newtoggle{IHpresent} % Internal Hardcopy
  \newtoks\EHtoks\newtoggle{EHpresent} % External Hardcopy
  \newtoks\IStoks\newtoggle{ISpresent} % Internal Softcopy
  \newtoks\EStoks\newtoggle{ESpresent} % External Softcopy

  % These are utility macros to force the token expansion to happen
  % correctly
  \newcommand*\addEHtoks[1]{\global\EHtoks\expandafter{\the\EHtoks##1}}
  \newcommand*\addEStoks[1]{\global\EStoks\expandafter{\the\EStoks##1}}
  \newcommand*\addIHtoks[1]{\global\IHtoks\expandafter{\the\IHtoks##1}}
  \newcommand*\addIStoks[1]{\global\IStoks\expandafter{\the\IStoks##1}}

  % We try to map the semantics of the original SANDdist commands into the new categories. Fragile and
  % probably will need refinement.
  \newcommand{\SANDdistInternal}[4]{
    \ifthenelse{\boolean{SANDdistributionProvided}}   {
    }{
      \ClassError{SANDreport.cls}{\@backslashchar SANDdistribution
        not (yet) provided}{\@backslashchar SANDdistribution must
        be used before any \@backslashchar SANDdistInternal}
    }
    \toggletrue{IHpresent}
    \addIHtoks{##1 & ##3 & ##4 & ##2 \\\hline}
  }
  
  \newcommand{\SANDdistInternalEmail}[3]{
    \ifthenelse{\boolean{SANDdistributionProvided}}   {
    }{
      \ClassError{SANDreport.cls}{\@backslashchar SANDdistribution
        not (yet) provided}{\@backslashchar SANDdistribution must
        be used before any \@backslashchar SANDdistInternal}
    }
    \toggletrue{ISpresent}
    \addIStoks{##1 & ##2 & ##3 \\\hline}
  }
  
  % Use a mail channel? Unclear if this is needed, and currently is a noop.
  \newcommand{\SANDdistInternalM}[4]{
    \ifthenelse{\boolean{SANDdistributionProvided}}   {
    }{
      \ClassError{SANDreport.cls}{\@backslashchar SANDdistribution
        not (yet) provided}{\@backslashchar SANDdistribution must
        be used before any \@backslashchar SANDdistInternal}
    }
  }
  
  \newcommand{\SANDdistExternal}[3]{
    \ifthenelse{\boolean{SANDdistributionProvided}}   {
    }{
      \ClassError{SANDreport.cls}{\@backslashchar SANDdistribution
        not (yet) provided}{\@backslashchar SANDdistribution must
        be used before any \@backslashchar SANDdistExternal}
    }
    \toggletrue{EHpresent}
    % parbox used here because names and physical address might have embedded newlines
    \addEHtoks{##1 & \parbox{2.5in}{\centering##2} & \parbox{2.5in}{##3} \\\hline }
  }
  
  \newcommand{\SANDdistExternalEmail}[3]{
    \ifthenelse{\boolean{SANDdistributionProvided}}   {
    }{
      \ClassError{SANDreport.cls}{\@backslashchar SANDdistribution
        not (yet) provided}{\@backslashchar SANDdistribution must
        be used before any \@backslashchar SANDdistExternal}
    }
    \toggletrue{ESpresent}
    \addEStoks{##1 & ##2 & ##3 \\\hline}
  }

}{
  %
  % at \end{SANDdistribution}
  %
  \clearpage
  \section*{DISTRIBUTION}

  \ifthenelse{\boolean{distCA}}   {
    % Insert the standard CA distribution
    
    % If this report is for LDRD work
    \iftoggle{distLDRD}{
      \SANDdistInternal{1}{0359}{L. Martin, LDRD Office}{1910}
    }{}

    % If this report is for CRADA work
    \iftoggle{distCRADA}{
      \SANDdistInternal{1}{0115}{OFA/NFE Agreements}{10112}
    }{}

    % If this report has a Patent Caution or Patent Interest
    \iftoggle{distPatent}{
      \SANDdistInternal{1}{0161}{Legal Intellectual Property}{11500}
    }{}

    \iftoggle{distClassified}{
      \SANDdistInternal{2}{9018}{Central Technical Files}{8944 (1 electronic and 1 hardcopy)}
    }{}
    \SANDdistInternalEmail{CA Technical Library}{8551}{cateclib@sandia.gov}

  }{}

  \ifthenelse{\boolean{distNM}}   {
    % Insert the standard NM distribution

    % If this report is for LDRD work
    \iftoggle{distLDRD}{
      \SANDdistInternal{1}{0359}{L. Martin, LDRD Office}{1910}
    }{}

    % If this report is for CRADA work
    \iftoggle{distCRADA}{
      \SANDdistInternal{1}{0115}{OFA/NFE Agreements}{10112}
    }{}

    % If this report has a Patent Caution or Patent Interest
    \iftoggle{distPatent}{
      \SANDdistInternal{1}{0161}{Legal Intellectual Property}{11500}
    }{}

    \iftoggle{distClassified}{
      \SANDdistInternalM{2}{2497}{Central Technical Files}{8944 (1 electronic and 1 hardcopy)}
    }{}

    \SANDdistInternalEmail{Technical Library}{1911}{sanddocs@sandia.gov}

  }{}

  % 
  % Assuming we have collected all the distribution items, we can now build the tables
  %
  
  % borrowed a little latex-fu to set up the table columns from
  % https://tex.stackexchange.com/questions/12703/how-to-create-fixed-width-table-columns-with-text-raggedright-centered-raggedlef
  \newcolumntype{D}[1]{>{\centering\let\newline\\\arraybackslash\hspace{0pt}}m{##1}}
  \renewcommand{\arraystretch}{1.5}
  \small\sffamily
  \par
  \iftoggle{EHpresent}{
    {\bfseries Hardcopy\textemdash{}External}\par
    \vspace{.015in}
    \begin{longtable}{|D{1in}|D{2.5in}|D{2.5in}|}
      \hline
      \rowcolor[gray]{.5}\bfseries Number of \newline Copies & \bfseries Name(s) & \bfseries Company Name
      and \newline Company Mailing Address \\\hline
      \endhead
      \the\EHtoks
    \end{longtable}
  }{}
  \par
  \iftoggle{IHpresent}{
    {\bfseries Hardcopy\textemdash{}Internal}\par
    \vspace{.015in}
    \begin{longtable}{|D{1in}|D{2.5in}|D{1in}|D{1in}|}
      \hline 
      \rowcolor[gray]{.5}\bfseries Number of \newline Copies & \bfseries Name & \bfseries Org. & \bfseries Mailstop \\\hline\endhead
      \the\IHtoks 
    \end{longtable}
  }{}
  \par
  \iftoggle{ESpresent}{
    {\bfseries Email\textemdash{}External (encrypt for OUO)}\par
    \vspace{.015in}
    \begin{longtable}{|D{2.0in}|D{2.0in}|D{2.0in}|} \hline 
      \rowcolor[gray]{.5}\bfseries Name & \bfseries Company Email \newline Address & \bfseries Company Name \\\hline\endhead
      \the\EStoks 
    \end{longtable}
  }{}
  \par
  \iftoggle{ISpresent}{
    {\bfseries Email\textemdash{}Internal (encrypt for OUO)}\par
    \vspace{.015in}
    \begin{longtable}{|D{2.5in}|D{1in}|D{2.5in}|} \hline 
      \rowcolor[gray]{.5}\bfseries Name & \bfseries Org. & \bfseries Sandia Email Address \\\hline\endhead
      \the\IStoks 
    \end{longtable}
  }{}
  
}



% 
% The distribution list for a reprint
% No housekeeping copies are needed.
\newenvironment{SANDreDistribution}{
  \ifthenelse{\boolean{SANDrePrintDateProvided}}   {
    \setboolean{SANDdistributionProvided}{true}

    \stepcounter{SANDreDistCnt}
    \ifthenelse{\value{SANDreDistCnt} = 2}   {
      \sbox{\SANDrePrintDistBox}{\usebox{\SANDrePrintDistBoxSecond}}
    }{
      \ifthenelse{\value{SANDreDistCnt} = 3}   {
        \sbox{\SANDrePrintDistBox}{\usebox{\SANDrePrintDistBoxThird}}
      }{
        \ifthenelse{\value{SANDreDistCnt} = 4}   {
          \sbox{\SANDrePrintDistBox}{\usebox{\SANDrePrintDistBoxFourth}}
        }{
          \ifthenelse{\value{SANDreDistCnt} = 5}   {
            \sbox{\SANDrePrintDistBox}{\usebox{\SANDrePrintDistBoxFifth}}
          }{
            \ClassError{SANDreport.cls}{\@backslashchar SANDreDistribution
              used more than four times!}{Do you really have more than
              five printings? If so, you need to amend SANDreport.cls}
          }
        }
      }
    }

    \usebox{\SANDrePrintDistBox}
    \normalsize
    \raggedbottom
    \begin{SANDdescription}
    }{
    }
  }{
    \ifthenelse{\boolean{SANDrePrintDateProvided}}   {
    \end{SANDdescription}
  }{
  }
}


\ifthenelse{\boolean{relaxedSAND}}{
}{
  \AtEndDocument{
    % Do some more usage checking
    \ifthenelse{\boolean{SANDmainProvided}}   {
    }{
      \ClassError{SANDreport.cls}{\@backslashchar SANDmain
        not provided}{Add \@backslashchar SANDmain before your
        introduction (first) section of your document}
    }
    
    % background image layer to apply to cover page
    \DeclareNewLayer[background,page, mode=picture, contents={%
      \putLL{\includegraphics{\SANDbackcoverBackground}}}]{SANDbackcover-bg}
    
    \DeclareNewLayer[foreground,mode=text,area={1.1in}{8.8in}{1.5in}{2in},contents={%
      \raggedright\sffamily\scriptsize Sandia National Laboratories is a multimission laboratory
      managed and operated by National Technology \& Engineering Solutions of Sandia LLC, a wholly owned
      subsidiary of Honeywell International Inc., for the U.S. Department of Energy's National Nuclear
      Security Administration under contract DE-NA0003525.\vfill}]{SANDbackcover-mission}
    
    % Put the Sandia Logo on the back page. Historically for SAND reports this has been required to be
    % an even page to accomdate two-sided printed copies.
    %
    \cleardoubleevenemptypage % KOMA-script for always skipping to next even page.
    
    \setlength{\footskip}{1.2\footskip} % reset this length for the back cover
    \DeclarePageStyleByLayers{SANDbackcover-recto}{SANDbackcover-bg,SANDbackcover-mission,SANDcover-header,SANDcover-footer}
    \thispagestyle{SANDbackcover-recto}
    \null
    
  } % End of AtEndDocument{}
}


%
% These are included for compatibility with the existing set of examples and for people who haven't moved
% off their usage
%
\DeclareOldFontCommand{\rm}{\normalfont\rmfamily}{\mathrm}
\DeclareOldFontCommand{\sf}{\normalfont\sffamily}{\mathsf}
\DeclareOldFontCommand{\tt}{\normalfont\ttfamily}{\mathtt}
\DeclareOldFontCommand{\bf}{\normalfont\bfseries}{\mathbf}
\DeclareOldFontCommand{\it}{\normalfont\itshape}{\mathit}
\DeclareOldFontCommand{\sl}{\normalfont\slshape}{\@nomath\sl}
\DeclareOldFontCommand{\sc}{\normalfont\scshape}{\@nomath\sc}
