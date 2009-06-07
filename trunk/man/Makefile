all: vmpk.1

vmpk.1: vmpk.xml
	# Verification
	-xmllint --nonet --noout --postvalid --xinclude $^
# Compilation
	xsltproc --output $@ --nonet --xinclude \
	  /usr/share/xml/docbook/stylesheet/nwalsh/current/manpages/docbook.xsl \
	  $^
