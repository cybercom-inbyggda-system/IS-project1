#!/usr/bin/env perl

# Copyright (c) 2017 Cybercom Sweden AB
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

#
# This script converts an image to hex code intended for the OLED display SSD1306.
#

use warnings;
use strict;

use GD;

my $file_name = $ARGV[0];
die "Need image file as parameter" if (not defined $file_name);

GD::Image->trueColor(1);
my $image = new GD::Image($file_name) or die "Could not open file \"$file_name\"";

my $width = $image->width;
my $height = $image->height;

my $round_off_limit = 3*220; #255 * 3 / 2;
my $invert_black_white = 1; # 0 or 1

printf "//   Image: $file_name\n";

# print picture with frame
printf "//   +";
for (my $y = 0; $y < $width; $y++) { printf "-"; }
printf "+\n";
for (my $y = 0; $y < $height; $y++)
{
    printf "//   |";
    for (my $x = 0; $x < $width; $x++)
    {
	my $index = $image->getPixel($x, $y);
	my ($r,$g,$b) = $image->rgb($index);
	if (($r + $g + $b < $round_off_limit) == $invert_black_white) { printf "*"; }
	else { printf " "; }
    }
    printf "|\n";
}
printf "//   +";
for (my $y = 0; $y < $width; $y++) { printf "-"; }
printf "+\n";

printf "// Width:  $width\n";
printf "// Height: $height\n";

# do image -> hex
for (my $y_base = 0; $y_base < $height; $y_base+=8) # y stepped in chunks of 8-bits
{
    my $y_offset_len = 8;
    $y_offset_len = $height - $y_base if $y_base + 8 > $height;

    printf "// pixels row %u - %u:\n", $y_base, $y_base + $y_offset_len - 1;
    for (my $x = 0; $x < $width; $x++)
    {
	my $value = 0;
	for (my $y_offset = 0; $y_offset < $y_offset_len; $y_offset++)
	{
	    my $y = $y_base + $y_offset;
	    my $index = $image->getPixel($x, $y);
	    my ($r,$g,$b) = $image->rgb($index);

	    $value |= 1 << $y_offset if (($r + $g + $b < $round_off_limit) == $invert_black_white);
	}
	printf "0x%02x,", $value;
	printf "\n" if ($x % 16 == 15 || $x == $width - 1);
    }
}
