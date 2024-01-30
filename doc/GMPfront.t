.ig
	front.t
..
.nr PDFOUTLINE.FOLDLEVEL 1
.defcolor pdf:href.colour rgb 0.00 0.25 0.75
.pdfinfo /Title "The Groff Manpage Book"
.de an*cln
.  ds \\$1
.  als an*cln:res \\$1
.  shift
.  ds an*cln:res \\$*\"
.  ds an*cln:chr \\$*
.  substring an*cln:chr 0 0
.  if '\%'\\*[an*cln:chr]' \{\
.    substring an*cln:res 1
.  \}
..
.
.de END
..
.
.am reload-man END
.de an*bookmark
.  if '\\\\*[.T]'pdf' \{\
.    ie (\\\\$1=1) \{\
.       an*cln an*page-ref-nm \\\\$2\"
.       pdfbookmark -T "\\\\*[an*page-ref-nm]" \\\\$1 \\\\$2
.    \}
.    el .pdfbookmark \\\\$1 \\\\$2
.  \}
..
.
.de1 MR
.  if ((\\\\n[.$] < 2) : (\\\\n[.$] > 3)) \
.    an-style-warn .\\\\$0 expects 2 or 3 arguments, got \\\\n[.$]
.  if '\\\\*[.T]'pdf' \{\
.    ie \\\\n(.$=1 \
.      I \\\\$1
.    el \{\
.      an*cln an*page-ref-nm \\\\$1(\\\\$2)
.      ie d pdf:look(\\\\*[an*page-ref-nm]) .pdfhref L -D \\\\*[an*page-ref-nm] -A "\\\\$3" -- \fI\\\\$1\fP(\\\\$2)
.      el .IR \\\\$1 (\\\\$2)\\\\$3
.    \}
.  \}
.  hy \\\\n[an*hyphenation-mode]
..
.END
.
.de Hl
.br
\l'\\n[.l]u-\\n[.i]u\&\\$1'
.br
..
\Z@\D't 8p'@
.pdfbookmark 1 Cover
.pdfpagenumbering
.sp 2i
.Hl
.sp .6i
.ad r
.ps 52
\m[maroon]Groff\m[]
.sp 18p
.ps 16
\f[BMB]THE MAN PAGES BOOK\fP
.sp .2i
.Hl
.pn 1
.bp
.pdfpagenumbering D . 1
