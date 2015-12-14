#!/bin/sh

rename "s/ - //" *.png
mogrify -trim *.png
