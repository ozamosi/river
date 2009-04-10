SUBDIRS = \
	river \
	data \
	plugins

dist-hook:
	python gen-changelog.py > ChangeLog

EXTRA_DIST = \
	intltool-extract.in \
	intltool-update.in \
	intltool-merge.in

DISTCLEANFILES = \
	intltool-extract \
	intltool-update \
	intltool-merge \
	po/.intltool-merge-cache
