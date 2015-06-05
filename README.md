# TiffHDR

## Presentation

TiffHDR is a software for merging TIFF photos in order to  produce
an HDR picture, still in TIFF and uncompressed so that it may be
further retouched.

Bear in mind that your photos are supposed to be aligned
(use a tripod) and that this program does not provide any tool
for correcting alignment.

It shall support 16 bit channels, but it is still in
developpement; please come back in a few weeks.

## Motivation

After having tried hdrmerge and Luminance HDR,
which both failed to merge my photos,
I decided to program my own software.
To be able to handle more than 8-bits per channel,
and yet no go through the trouble of handling all the existing RAW formats,
I chose to load TIFF files that may have been generated from RAW files
with RawTherapee.

