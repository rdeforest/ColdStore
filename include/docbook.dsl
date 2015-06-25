<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY % html "IGNORE">
<![%html;[
<!ENTITY % print "IGNORE">
<!ENTITY docbook.dsl SYSTEM "/usr/lib/sgml/stylesheet/dsssl/docbook/nwalsh/html/docbook.dsl" CDATA dsssl>
]]>
<!ENTITY % print "INCLUDE">
<![%print;[
<!ENTITY docbook.dsl SYSTEM "/usr/lib/sgml/stylesheets/dssl/docbook/nwalsh/print/docbook.dsl" CDATA dsssl>
]]>
]>
<style-sheet>
<style-specification id="print" use="docbook">
<style-specification-body>

;; customize the print stylesheet

(define %visual-acuity%
  ;; REFENTRY
  ;; PURP General measure of document text size
  ;; DESC
  ;; This parameter controls the general size of the text in the document.
  ;; Several other values (body font size and margins) have default values that
  ;; vary depending on the setting of '%visual-acuity%'. There
  ;; are three legal values: 'normal', 
  ;; the normal, standard document size (10pt body text);
  ;; 'presbyopic', 
  ;; a slightly more generous size (12pt body text); and
  ;; 'large-type',
  ;; quite large (24pt body text).
  ;; /DESC
  ;; AUTHOR N/A
  ;; /REFENTRY
  ;; "presbyopic"
  ;; "large-type"
  "presbyopic")

(define %paper-type%
  ;; REFENTRY
  ;; PURP Name of paper type
  ;; DESC
  ;; The paper type value identifies the sort of paper in use, for example, 
  ;; 'A4' or 'USletter'. Setting the paper type is an
  ;; easy shortcut for setting the correct paper height and width.
  ;; 
  "A4")

(define %number-address-lines% 
  ;; REFENTRY
  ;; PURP Enumerate lines in a 'Address'?
  ;; DESC
  ;; If true, lines in each 'Address' will be enumerated.
  ;; See also '%linenumber-mod%', '%linenumber-length%',
  ;; '%linenumber-padchar%', and '($linenumber-space$)'.
  ;; /DESC
  ;; AUTHOR N/A
  ;; /REFENTRY
  #t)

</style-specification-body>
</style-specification>

<style-specification id="html" use="docbook">
<style-specification-body>

;; customize the html stylesheet

(define %gentext-nav-tblwidth%
  ;; If using tables for navigation, how wide should the tables be?
  "100%")

(define %html-pubid%
  ;; What public ID are you declaring your HTML compliant with?
  "-//W3C//DTD HTML 4.0//EN")

(define %stylesheet-type%
  ;; The type of the stylesheet to use
  "text/css")

(define %stylesheet%
  ;; Name of the stylesheet to use
  "../include/docbook.css")

(define %body-attr% 
  ;; What attributes should be hung off of BODY?
  (list
   (list "BGCOLOR" "#FFFFFF")
   (list "TEXT" "#000000")))


(define %gentext-nav-use-tables%
  ;; Use tables to build the navigation headers and footers?
  #t)

(define %generate-part-toc%
  ;; generate a TOC even for a single file
 #t)

(define %shade-verbatim%  
  ;; REFENTRY
  ;; PURP Should verbatim environments be shaded?
  ;; DESC
  ;; If true, a table with '%shade-verbatim-attr%' attributes will be
  ;; wrapped around each verbatim environment.  This gives the effect
  ;; of a shaded verbatim environment.
  ;; /DESC
  ;; AUTHOR N/A
  ;; /REFENTRY
  #t)

(define %shade-verbatim-attr% 
  ;; REFENTRY
  ;; PURP Attributes used to create a shaded verbatim environment.
  ;; DESC
  ;; See '%shade-verbatim%'
  ;; /DESC
  ;; AUTHOR N/A
  ;; /REFENTRY
  (list
   (list "BORDER" "0")
   (list "BGCOLOR" "#E0E0E0")
   (list "WIDTH" "80%")))

(define %root-filename%
  ;; Name for the root HTML document
  "index")

</style-specification-body>
</style-specification>

<external-specification id="docbook" document="docbook.dsl">
</style-sheet>
