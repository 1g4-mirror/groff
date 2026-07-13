.\" Run: pdfmom --roff -ms -msboxes -dpapersz=a4 -tU -P-pa4  Groff-PDF-Features.ms > Groff-PDF-Features.pdf
.\"
.\" Copyright 2026 Free Software Foundation, Inc.
.\"
.\" This file is part of groff, the GNU roff typesetting system.
.\"
.\" groff is free software; you can redistribute it and/or modify it
.\" under the terms of the GNU General Public License as published by
.\" the Free Software Foundation, either version 3 of the License, or
.\" (at your option) any later version.
.\"
.\" groff is distributed in the hope that it will be useful, but WITHOUT
.\" ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
.\" or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
.\" License for more details.
.\"
.\" You should have received a copy of the GNU General Public License
.\" along with this program. If not, see <http://www.gnu.org/licenses/>.
.nr PS 11
.nr VS 13
.nr PO 2c
.nr LL 17c
.nr PL 29.7c
.nr HM 1.9c
.nr FM .3c
.nr QI 5n
.nr DI \n[QI]
.nr PDFOUTLINE.FOLDLEVEL 1
.ds FR 1
.nr TC-MARGIN \w'00' \" expect 2-digit page numbers at most
.ie t .nr PI 3.5n
.el   .nr PI 4n
.ds date \*[MONTH\n[mo]] \n[year]\"
.ND \*[date]
.EH '%''\*[date]'
.EF ''''
.OH '\f[I]Groff\f[] PDF features''%'
.OF ''''
.de Cd
.  ft CB
..
.de CdX
.  ft
..
.de Qq
.  nop \[lq]\\$1\[rq]\\$2
..
.\" Define a macro for code literals; use bold and disable hyphenation.
.de Lt
.  ft CB
.  nh
.  nop \m[grey33]\s'-1p'\\$1\s'+1p'\m[]\c
.  hy \\n[HY]
.  ft
.  nop \&\\$2
..
.de PN
.  ev Pn
.  evc 0
.  po .5c
.  pdfnote \\$@
.  fl
.  po
.  ev 0
..
.ds = \f(CB\\$1\f(CR\\$4\f[CBI]\\$2\f(CR\\$3
.RP no
.TL
.BI Groff
PDF features
.AU
Deri James
.AI
deri@chuzzlewit.myzen.co.uk
.\" .AB no
.\" \X'pdf: pdfpic gnu.eps -C 0 0 15c'
.\" .sp 5c
.\" .AE
.NH 1 intro
.XN Introduction
.LP
This is an attempt to draw together information gleaned from
.Lt pdf.tmac ,
.Lt gropdf(1) ,
and the various full service macros which manage pdf features.
.LP
Previously groff contained
.Lt pdfmark.tmac
and
.Lt spdf.tmac
(to integrate pdf features into the
.Lt ms
macros). Since these have now been dropped from groff and are now supported by
Keith Marshall at
.pdfhref W -D https://savannah.nongnu.org/projects/groff-pdfmark -A . this site
.LP
PDF features can either be controlled at a low level,
using the macros defined in
.Lt pdf.tmac
or rely on these
macros being integrated into a full-service macro
package. Currently this is the state of play with regard
to full-service:-
.LP
.TS
allbox tab(^);
Cb Cb Cb Cb Cb Cb
Cf(CB) C C C C C.
Macro^Meta Data^Bookmarks^Internal Link^External Link^Named Destination
mom^\[OK]^\[OK]^\[OK]^\[OK]^\[OK]
man^^\[OK]^\[OK]\m[red]\**\m[]^\[OK]^
ms^\[OK]^\[OK]^\[OK]^^\[OK]
me^^^^^
mm^^^^^
.TE
.LP
.FS
In a man page collection the MR macro can be used to link to other entries
in the collection, otherwise it is an external link.
.FE
.LP
The macros outlined below cannot be mix'n'matched between full service
macro sets, but the information given for
.Lt pdf.tmac
can be used in any document, particularly if a full macro set is missing
a particular facility.
.LP
.NH 1 meta
.XN Meta Data
.NH 2 info
.XN Document Info
.LP
A PDF document can contain meta data such as
.Qq "Created Date" ,
.Qq Author ,
.Qq Title ,
etc..
.NH 3
.XN pdf.tmac
.BOXSTART SHADED cornsilk OUTLINED brown INDENT 2n WEIGHT 1p
.BOXSTART SHADED cornsilk3 INDENT 2p
.ce 1
.Lt pdf.tmac
.BOXSTOP
.LP
To enter pdf meta data use:
.QS
.Lt ".pdfinfo /<label>"
"\fItext\fP"
.QE
Where
.Lt /<label>
can be:-
.LP
.QS
.Lt /Title ,
.Lt /Author ,
.Lt /Subject ,
.Lt /Keywords
.QE
And
.BI text
can be multi-line if you use:-
.QS
.Lt \[rs]*[PDFLB]
where you want the line-break.
.QE
These entries are stored in the pdf and can be seen if you run the
command:-
.QS
.Lt pdfinfo
\fIfilename\fP
.QE
Or are probably visible if you look at
.Qq "Document Properties"
in your favourite pdf viewer.
.LP
.BOXSTOP
.NH 3
.XN mom
.BOXSTART SHADED lavenderblush OUTLINED mediumblue INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt mom
.BOXSTOP
.LP
In
.Lt mom
you can use:-
.QS
.Lt .DOCTITLE
.BI text
.LP
or more usually
.LP
.Lt .PDF_TITLE
.BI text
.QE
The text given to:-
.QS
.Lt .AUTHOR
.BI text
.QE
Will be added to the pdf meta data.
.BOXSTOP
.LP
.NH 3
.XN ms
.BOXSTART SHADED honeydew OUTLINED green INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt ms
.BOXSTOP
.LP
.Lt .TL
and
.Lt .AU
Provide the Title and Author for the pdf.
.BOXSTOP
.LP
.NH 2 notes
.XN Annotation Notes
.LP
.PN -T "Deri James" Just an example
These are the clickable/hoverable icons which appear in the document which
can contain pop-up comments on the text. Only the
.Lt pdf.tmac
macros provide access to this facility.
.LP
.NH 3
.XN pdf.tmac
.BOXSTART SHADED cornsilk OUTLINED brown INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt pdf.tmac
.BOXSTOP
.LP
.Lt .pdfnote
[ -T
.BI title
]
.BI "text"
.LP
The popup note in the left margin was created with:-
.QS
.Lt \&.pdfnote \"
-T "Deri James" Just an example
.QE
The colour of the icon is set by setting the register
.Lt PDFNOTE.COLOUR
(or
.Lt PDFNOTE.COLOR )
to three numbers each in the range 0.0 \[rA] 1.0 representing red, green,
blue values, so:-
.QS
\&.ds
.Lt PDFNOTE.COLOUR
1.0 1.0 0.0
.QE
Produces a yellow icon. The opacity of the icon is controlled by:-
.QS
\&.ds
.Lt PDFNOTE.OPACITY
0.0 \[rA] 1.0
.QE
Where the default value is 0.6.
.LP
Again you can use
.Lt \[rs]*[PDFLB]
in the
.BI text .
.BOXSTOP
.LP
.NH 1 bookmarks
.XN Bookmarks
.LP
For PDFs, bookmarks are the entries in the outline panel. Each bookmark
has a hierarchical level, and, optionally, can be
.Qq named
so that they can be linked to from elsewhere in the document
.XR naming ). (
.LP
Bookmarks in the panel can be open or closed, setting:-
.QS
\&.ds
.Lt PDFOUTLINE.FOLDLEVEL
1
.QE
will
.Qq close
all bookmarks below the first level. The default is to open
all bookmarks.
.NH 2
.XN pdf.tmac
.BOXSTART SHADED cornsilk OUTLINED brown INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt pdf.tmac
.BOXSTOP
.LP
.Lt pdf.tmac
defines this macro for bookmarks:-
.QS
.Lt .pdfbookmark
[\c
.Lt -T
.BI name ]
.BI level
.BI "descriptive text ..."
.QE
Where
.Qq level
is the mandatory nesting level for this
.Qq bookmark .
All the
.Qq "descriptive text"
form the entry in the outline panel. If the
.Lt -T
flag is given this becomes a
.Qq "named destination"
which can be linked to from elsewhere in the document
.ds here naming
.XR naming ). (
.ds here after
.BOXSTOP
.ds here ne
.ne 5
.ds here
.NH 2
.XN mom
.BOXSTART SHADED lavenderblush OUTLINED mediumblue INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt mom
.BOXSTOP
.LP
mom uses the
.Lt .HEADING
macro to populate the outline panel:-
.QS
.Lt .HEADING
.BI level
[\c
.Lt NAMED
.BI name ]
.BI "descriptive text ..."
.QE
.Qq level ,
.Qq name
and
.Qq "descriptive text"
have the same meanings as above.
.BOXSTOP
.LP
.NH 2
.XN ms
.BOXSTART SHADED honeydew OUTLINED green INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt ms
.BOXSTOP
.LP
There are two separate ways of providing outline bookmarks in
.Lt ms .
First there is the traditional heading:-
.QS
.Lt .NH
.BI level
[\c
.BI name \m[red]\**\m[]]
.QE
.FS
If the
.Qq level
is
.Qq S
(a Berkeley extension) you cannot have a
.Qq named
bookmark.
.FE
This is then followed by input which is used as
.Qq "descriptive text" .
.LP
Although there could be several lines of input text, it is the
accumulated output line which is used as
.Qq "descriptive text" .
.LP
.pdfhref M -N hdexample ms Heading Example
So this input:
.ID
.ft CB
\&.NH 1 using
Using
\&.BI groff
with the
\&.BI ms
Macro Package
\&.XS
Using
\&.BI groff
with the
\&.BI ms
Macro Package
\&.XE
.ft
.DE
Would create a named (\c
.Qq using )
level 1 bookmark
.Qq "Using \fIgroff\fP with the \fIms\fP Macro Package" ,
and also create a
.Lt TOC
entry.
Note if the input text results in multiple output lines it is
only the first output line which is used as the bookmark text.
.LP
.pdfhref M -N XN Using .XN
The second method, which combined the
.Qq heading
with populating a
.Lt TOC ,
is the
.Lt .XN
command which
.B must
immediately follow the
.Lt .NH
command:-
.QS
.Lt .XN
.BI "descriptive text"
.QE
Previously the
.Lt .NH
and following heading text had to be followed with
.Lt .XS/.XE
commands with the same text to separately populate the
.Lt TOC .
.LP
So the equivalent code to the above using
.Lt .XN
is:-
.ID
.ft CB
\&.NH 1 using
\&.XN Using \[rs]fIgroff\[rs]fP with the \[rs]fIms\[rs]fP Macro Package
.ft
.DE
In both cases the bookmark is named
.Qq using ,
and clicking on the overview panel
or the hotspot link in the
.Lt TOC
will jump to the correct part of the document.
You can also use the
.XR XR "" "" "\&.XR command"
to jump to the section named
.Qq using :-
.QS
\&.XR using ). (
.QE
to enter this hotlink
.Qq "(see: Using \fIgroff\fP with the \fIms\fP Macro Package)" .
.BOXSTOP
.bp
.NH 2
.XN man
.LP
.BOXSTART SHADED linen OUTLINED maroon INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt man
.BOXSTOP
.LP
The
.QS
.Lt .TH
.BI identifier
.BI section
.QE
macro creates (via first call to
.Lt .PT )
a named heading with name id of
.Qq identifier(section) .
It is normally given a level 1 bookmark, which can be controlled by setting
.Qq an*bookmark-base-level
on the command line to increase the level,
this can be used in man page collections where the pages
are split into separate sections.
.LP
The
.Lt .SH
and
.Lt .SS
macros create unnamed bookmarks at the appropriate level.
.BOXSTOP
.LP
.NH 1 naming
.XN Named Destination
.LP
In order to insert hotspot links which jump to other parts of the
document, the destination must be
.Qq named .
In the section on
.XR bookmarks "" "" bookmarks
it showed how they can be
.Qq named .
.LP
If you want to jump to a place in the document which has not got a bookmark,
you can name any particular place in the document, for example, a table or a
figure to which you may want to refer. See information below.
.NH 2
.XN pdf.tmac
.LP
.BOXSTART SHADED cornsilk OUTLINED brown INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt pdf.tmac
.BOXSTOP
.LP
.pdfhref M -N pdfhrefM
This macro will plant a named destination without inserting a bookmark.
.QS
.Lt ".pdfhref M"
[\c
.Lt -N
.BI name ]
[\c
.Lt -E ]
.BI "descriptive text" ] [
.QE
If no
.BI name
is specified, the first word of
.Qq "descriptive text"
is used as the
.Qq name ,
which means that if the
.Lt -N
flag is not used then there must be at least one word in the text.
.LP
The
.Qq "descriptive text" ,
if any, is saved as the
.Qq value
of the destination, and can be used when specifying a hotspot link
.XR hotspot ). (
.LP
If the
.Lt -E
flag is used, the text is output to the document as well.
.BOXSTOP
.LP
.NH 2
.XN mom
.BOXSTART SHADED lavenderblush OUTLINED mediumblue INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt mom
.BOXSTOP
.LP
The mom macro:-
.QS
.Lt .PDF_TARGET
.BI name
[\c
.BI "descriptive text" ]
.QE
You can now use
.BI name
as the destination of any hotspot link
.XR hotspot ). (
.BOXSTOP
.LP
.NH 2
.XN ms
.BOXSTART SHADED honeydew OUTLINED green INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt ms
.BOXSTOP
.LP
The
.I ms
macro
.QS
.Lt .XM
.BI name
[\c
.BI "descriptive text" ]
.QE
Marks a position in the document to which
.Lt .XR
can place a hotspot link
.XR hotspot ). (
.BOXSTOP
.bp
.NH 1 hotspot
.XN Internal Hotspot Links
.LP
An internal link is to a
.Qq "named destination"
within the document,
.XR external "" "" external
are links using a URI.
.LP
A hotspot link is a clickable piece of text which will cause the viewer
to jump to a
.XR naming . "" *
The colour of the hotspot is controlled by setting the register
.Lt PDFHREF.COLOUR
(or
.Lt PDFHREF.COLOR )
to three numbers each in the range 0.0 \[rA] 1.0 representing red, green,
blue values, so:-
.QS
\&.ds
.Lt PDFHREF.COLOUR
0.00 0.35 0.60
.QE
Uses a turquoise text, which can be referenced as
.QS
.Lt \[rs]m[\[rs]*[PDFHREF.TEXT.COLOUR]] .
.QE
.ne 5
Hotspots can have a border drawn around them, which is controlled by setting
the register
.Lt PDFHREF.BORDER
to an array of 3 numbers which represent \[en] horizontal corner radius,
vertical corner radius, and border width. A zero radius gives square (not
rounded) corners. The default is:-
.QS
\&.ds
.Lt PDFHREF.BORDER
0 0 0
.QE
Which produces no border around the hotspot.
.NH 2
.XN pdf.tmac
.LP
.BOXSTART SHADED cornsilk OUTLINED brown INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt pdf.tmac
.BOXSTOP
.LP
To plant a hotspot link:-
.QS
.Lt ".pdfhref L " [\c
.Lt -D
.BI name ]
[\c
.Lt -P
.BI prefix-text ]
[\c
.Lt -A
.BI affixed-text ]
[\c
.Lt -- ]
[\c
.BI "hotlink text" ]
.QE
The target for the link is given by
.BI name
which should be a destination you have named
.XR naming ). (
.LP
.XM prefix Prefix and Affix
The
.BI prefix-text
and
.BI affixed-text
are placed around the
.BI "hotlink text" .
.LP
If you used a
.BI "descriptive text"
when naming a destination, you can retrieve the value with this code:-
.ID
.ft CB
\&.pdf:lookup \c
.BI name
\&.ie !'\[rs]*[pdf:lookup-result]'' \\
\&. ds desc-txt \&\[rs]*[pdf:lookup-value]
\&.el \\
\&. ds desc-txt Unknown
.ft
.DE
.LP
.XM expandochars "expando char"
This technique is used by some of the full service macros to provide the
.Qq expando
characters
.Qq *
and
.Qq + \&.
When they are the final character in
.Qq "hotlink text"
the expando is replaced by
.BI desc-text ,
surrounded by double quotes if the expando character is
.Qq + .
.BOXSTOP
.LP
.NH 2
.XN mom
.BOXSTART SHADED lavenderblush OUTLINED mediumblue INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt mom
.BOXSTOP
.LP
mom uses this macro:-
.QS
.Lt .PDF_LINK
.BI name
[\c
.Lt PREFIX
.BI text ]
[\c
.Lt SUFFIX
.BI text ]
.BI "hotlink text"
.QE
If the
.BI "hotlink text"
is terminated with an
.XR expandochars "" "" *
it is replaced by the value of the
.BI "descriptive text"
when the target
.BI name
was created.
.BOXSTOP
.LP
.NH 2 hotspotms
.XN ms
.BOXSTART SHADED honeydew OUTLINED green INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt ms
.BOXSTOP
.LP
.XM XR
An addition to the ms macro set is the
.Lt .XR
command, originally found in some versions of Keith Marshall's
.Lt spdf.tmac ,
our version now uses
.XR expandochars "" "" expandos
.QS
.Lt .XR
.BI name
[\c
.BI post
[\c
.BI pre
[\c
.BI "hotlink text" "] ] ]"
.QE
If
.BI "hotlink text"
is missing, the contents of
.Lt \[rs]*[spdf:txt_default]
is used instead. This defaults to
.Qq "see:\ +" " "
so, with the use of the terminating expando, the code
.QS
.Lt .XR
intro ), (
.QE
would add a link
.XR intro ), (
if the first section of this document was:-
.ID
\&.NH 1 intro
\&.XN Introduction
.DE
.BOXSTOP
.LP
.NH 2
.XN man
.LP
.BOXSTART SHADED linen OUTLINED maroon INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt man
.BOXSTOP
.LP
The
.QS
.Lt .MR
.BI name
.BI section
.QE
macro normally creates an external link
.XR external ), (
suitable for linking to
another man page, but if the other man page it is calling is part
of the same man page collection, it creates an internal link within
the document.
.BOXSTOP
.LP
.NH 1 external
.XN External Hotspot Links
.LP
External links are often links to resources on the internet such as website
URLs.
.NH 2
.XN pdf.tmac
.LP
.BOXSTART SHADED cornsilk OUTLINED brown INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt pdf.tmac
.BOXSTOP
.LP
The macro to link to internet resources is:-
.QS
.Lt ".pdfhref W " [\c
.Lt -D
.BI URI ]
[\c
.Lt -P
.BI prefix-text ]
[\c
.Lt -A
.BI affixed-text ]
[\c
.Lt -- ]
[\c
.BI "hotlink text" ]
.QE
If a
.BI URI
is not specified the
.BI "hotlink text"
must be a valid
.BI URI
itself, and the complete
.BI URI
will be visible in the document.
If a
.BI URI
is given by specifying
.Lt -D
.BI URI
then only the
.BI "hotlink text"
will be visible, but clicking it will launch the
.BI URI .
.LP
The
.Lt -P
and
.Lt -A
flags operate in the same way as in
.Lt ".pdfhref L "
.XR prefix ). (
.BOXSTOP
.LP
.NH 2
.XN mom
.BOXSTART SHADED lavenderblush OUTLINED mediumblue INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt mom
.BOXSTOP
.LP
External links are supported by:-
.QS
.Lt .PDF_WWW_LINK
.BI URI
[\c
.Lt PREFIX
.BI text ]
[\c
.Lt SUFFIX
.BI text ]
.BI "hotlink text"
.QE
If no
.BI "hotlink text"
is given, the
.BI URI
is used as the
.BI "hotlink text" .
.XR expandochars "" "" Expandos
can be used as the last character of
.BI "hotlink text"
and will insert the
.BI URI .
.BOXSTOP
.ne 8v
.LP
.NH 2
.XN man
.LP
.BOXSTART SHADED linen OUTLINED maroon INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt man
.BOXSTOP
.LP
The macro
.QS
.Lt .MR
.BI name
.BI section
.QE
Generates an appropriate external
.BI URI
such as:-
.QS
man:/groff(1)
.QE
except if the link is to another man page in the same collection.
.BOXSTOP
.LP
.ne 5
.NH 1 toc
.XN Table of Contents
.LP
Some full service macro sets offer macros for building a Table of Contents,
with PDF features these TOCs have clickable entries.
.LP
.NH 2
.XN pdf.tmac
.BOXSTART SHADED cornsilk OUTLINED brown INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt pdf.tmac
.BOXSTOP
.LP
.Lt pdf.tmac
has no specific macros to build Table of Contents, but it does have two macros
which can insert a TOC into the appropriate position in the document:-
.QS
.Lt .pdfpagename
.BI name
.QE
This assigns a name to the current page being rendered which can then be used in
the second macro:-
.QS
.Lt .pdfswitchtopage
[\c
.BI when ]
.BI name
.QE
Where
.BI name
is a previously named page, and the optional
.BI when
can be
.Qq before
or
.Qq after ,
default is
.Qq before .
Alternatively the positions
.Qq top
and
.Qq bottom
can be used instead of a named page.
This macro should be used before a new page is started.
.BOXSTOP
.LP
.NH 2
.XN mom
.BOXSTART SHADED lavenderblush OUTLINED mediumblue INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt mom
.BOXSTOP
.LP
The macro
.QS
.Lt .AUTO_RELOCATE_TOC
.QE
(must come before
.Lt .START )
will position the TOC appropriately in the document.
.LP
Pease see the
.pdfhref W -D http://schaffter.ca/mom/momdoc/tables-of-contents.html#top -- mom documentation
for details of TOC generation, by default the TOC entries are clickable.
.BOXSTOP
.LP
.NH 2
.XN ms
.BOXSTART SHADED honeydew OUTLINED green INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt ms
.BOXSTOP
.LP
The traditional way of populating a TOC was to use the
.Lt .XS/.XA/.XE
macros.
.LP
Groff 1.23.0 introduced the
.Lt .XN
macro which combined producing headings
.XR hdexample ) (
with adding a TOC entry.
.LP
In either case the entry will be clickable.
.LP
If the document has an
.Lt .RP
entry, so a title page is produced, the TOC will be repositioned after the
Title Page.
.BOXSTOP
.LP
.ne 7v
.NH 1 slide
.XN Slideshow Presentations
.LP
The groff pdf driver understands how to create pdfs to be used for slideshow
presentations. An alternative is to use the postscript driver and gpresent
which is downloadable
.pdfhref W -D https://ftp.openbsd.org/pub/OpenBSD/distfiles/gpresent-2.5.tar.gz here
because the original
.pdfhref W -D https://www.bob.diertens.org/corner/useful/gpresent/ website
is having difficulty downloading files larger than 32kb.
.LP
groff documents written to work with
.Lt present.tmac
.I may
work with -Tpdf with minor alteration (and no need to run
.Lt presentps
and
.Lt ps2pdf ).
.LP
.NH 2
.XN pdf.tmac
.BOXSTART SHADED cornsilk OUTLINED brown INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt pdf.tmac
.BOXSTOP
.LP
There are just two macros which control the slide presentation:-
.QS
.Lt .pdftransition
.I feature
.I mode
.I duration
.I dimension
.I motion
.I direction
.I scale
.I bool
.QE
These parameters are explained in the
.pdfhref W -D man:/gropdf(1)#transition gropdf(1)
man page, many of them are not required!
.LP
There are two
.Qq events
which trigger a transition: when a slide is first drawn
(\c
.I feature
=
.Lt SLIDE )
and when a new element is added to the slide
.I feature "" (
=
.Lt BLOCK ).
Both can have a different transition
.I mode ,
such as
.Lt "\[dq]Split | Blinds | Box | Wipe | Dissolve | Glitter | R\[dq]"
all of which are explained (and some more!) in the man page.
.LP
Whenever you want the presentation to pause (because you have added a
new element to the slide) use the macro:-
.QS
.Lt .pdfpause
.QE
and the new element will appear using the current transition set for
.Lt BLOCK .
.BOXSTOP
.LP
.NH 2
.XN mom
.BOXSTART SHADED lavenderblush OUTLINED mediumblue INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt mom
.BOXSTOP
.LP
mom has full integration with pdf presentation mode, the documentation
is
.pdfhref W -D http://schaffter.ca/mom/momdoc/docprocessing.html#slides -A . here
.LP
In addition the mom example documents include
.Lt slide-demo.mom
and its resultant pdf, so should be included in the examples/mom directory
in the groff documentation.
.BOXSTOP
.LP
.NH 2
.XN ms
.BOXSTART SHADED honeydew OUTLINED green INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt ms
.BOXSTOP
.LP
An example of using
.Lt ms
to create a slideshow is
.pdfhref W -D http://chuzzlewit.co.uk/ms-slides.tgz -A . here
.BOXSTOP
.bp
.NH 1 boxes
.XN Boxes and PDF paper colour
.LP
PDF viewers usually show contents on a white background, the actual
background in the PDF is really transparent (so it appears white),
however it is possible to specify a colour for the page background.
.LP
In addition it is possible to specify framed coloured boxes on the page
to contain the running contents. These boxes will flow onto following
pages.
.LP
.NH 2
.XN pdf.tmac
.BOXSTART SHADED cornsilk OUTLINED brown INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt pdf.tmac
.BOXSTOP
.LP
The relevent macros are:-
.QS
.Lt .pdfbackground
.I cmd
.I left
.I top
.I right
.I bottom
.I weight
.br
.Lt ".pdfbackground off"
.br
.Lt ".pdfbackground footnote"
.I bottom
.QE
which produce a background rectangle on the page. The meanings of the
parameters are explained in the
.pdfhref W -D man:/gropdf(1)#boxes gropdf(1)
man page.
.BOXSTOP
.LP
.NH 2
.XN ms
.BOXSTART SHADED honeydew OUTLINED green INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt ms
.BOXSTOP
.LP
If you include the macro file
.Lt -msboxes
in the
.I groff
command, you will have access to these macros:-
.QS
.Lt .BOXSTART
.Lt SHADED
.I colour
.Lt OUTLINED
.I colour
.Lt INDENT
.I size
.Lt WEIGHT
.I size
.QE
begins a box, where the argument after
.Lt SHADED
gives the fill colour and that after
.Lt OUTLINED
the border colour. Omit the former to get a borderless filled box and the
latter for a border with no fill. The specified
.Lt WEIGHT
(line thickness) is used if the box is
.Lt OUTLINED .
.LP
.Lt INDENT
precedes a value that leaves a gap between the border and the contents
inside the box.
.LP
Each colour must be a defined groff colour name, and each size a valid
.I groff
numeric expression. The keyword/value pairs can be specified in any order.
.LP
Boxes can be stacked, so you can start a box within another box; usually
the later boxes would be smaller than the containing box, but this is
not enforced. When using
.Lt BOXSTART ,
the left position is the current indent minus the
.Lt INDENT
in the command, and the right position is the left position
(calculated above) plus the current line length and twice the indent.
.QS
.Lt BOXSTOP
.QE
takes no parameters. It closes the most recently started box at the
current vertical position after adding its
.Lt INDENT
spacing.
.BOXSTOP
.LP
.NH 2
.XN mom
.BOXSTART SHADED lavenderblush OUTLINED mediumblue INDENT 2n WEIGHT 1p
.BOXSTART SHADED lightgray INDENT 2p
.ce 1
.Lt mom
.BOXSTOP
.LP
mom has comprehensive control of boxes see the
.pdfhref W -D http://schaffter.ca/mom/momdoc/images.html#box-intro \
   -A . mom documentation
.BOXSTOP
.bp
.NH 1 developer
.XN For macro developers
.LP
If you want to add PDF features to your own macro packages here is some
information which may be helpful.
.BOXSTART SHADED cornsilk OUTLINED brown INDENT 2n WEIGHT 1p
.sp -1v
.NH 2 restart
.XN Stop/Start hotlink
.QS
.Lt .pdfmarksuspend/.pdfmarkresume
.QE
If there is a piece of
.Qq "hotlink text"
which appears right at the bottom of the page and continues across to the
start of the next page these macros can stop/restart the hotlink.
Typically they appear at the start of a page footer macro (to prevent the
footer text becoming part of the hotlink), and at the end of a page
header macro (to restart the hotlinking from the previous page).
.LP
.NH 2 pdfroff
.XN Forward References
.LP
Since groff is a single pass interpreter when it finds an
.XR hotspot "" "" *
it cannot plant a link if it has not
.Qq seen
the
.XR naming "" "" *
to which it links, and the link will appear as
\m[\*[PDFHREF.TEXT.COLOUR]]Unknown\m[]
resisting all mouse clicks - the same happens if the
.I destination
of the link is mis-typed and does not exist. The forward link problem can be
solved: instead of:-
.QS
.Lt "groff -Tpdf ..."
.QE
use:-
.QS
.Lt "pdfmom --roff ..."
.QE
The command was originally developed for satisfying forward references in
.I mom
files (hence the name), but with the
.I --roff
flag it drops its reliance on
.I mom
and you can place any macro package on the command line.
.NH 2 S
.XN Marking Hotlinks
.LP
The
.QS
.Lt .pdfhref
.I command
.QE
general purpose macro, supports these commands:-
.QS
.IP \*[= O]
(Outline) This is the command which
.XR bookmarks "" "" \fB.pdfbookmark\fP
calls.
.IP \*[= M]
(Mark) Setup a
.XR naming . "" +
.IP \*[= L]
(Local) Mark an
.XR hotspot . "" +
.IP \*[= W]
(WWW URI) Form an
.XR external . "" +
.QE
.LP
These
.Qq low-level
calls all expect
.Qq "descriptive text"
or
.Qq "hotlink text"
to be passed as part of the macro call. Sometimes this is not convenient,
consider the man macro pair
.Lt .MT/.ME
where
.ID
.ft CB
\&.MT jh@\[rs]:axis\[rs]:.se
J\[rs][o ad]rgen H\[rs][a ad]gg
\&.ME
.ft
.DE
The mail-to
.I URI
is on the
.Lt .MT
line, but the text to use for a
.I hotlink
is everything up to the following
.Lt .ME .
.LP
The
.Lt .pdfhref
macro allows an
.Lt -S
flag, not documented above, which just turns on
.I hotlinking
text. It is then up to the macro author to arrange for it to be turned off
at the appropriate point. In this example it should be
.Qq "turned off"
in the
.Lt .ME
macro.
.LP
The code to turn off the
.Qq hotlinking
and restore the text colour to what it was before the
.Qq hotlinking
started is:-
.ID
.ft CB
\[rs]X'pdf: markend'\[rs]m[\[rs]*[pdf:curcol]]
.ft
.DE
.BOXSTOP
.TC
