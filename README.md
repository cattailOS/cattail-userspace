# nKernel
[![GitHub license](https://img.shields.io/github/license/novafurry/nKernel)](https://github.com/novafurry/nKernel/blob/master/LICENSE)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/85b7b1d7493544fb8175f73686663979)](https://app.codacy.com/gh/novafurry/nKernel/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![GitHub contributors](https://img.shields.io/github/contributors/novafurry/nKernel)](https://github.com/novafurry/nKernel/graphs/contributors)
[![GitHub commit activity](https://img.shields.io/github/commit-activity/m/novafurry/nKernel)](https://github.com/novafurry/nKernel/commits)

The "kernel" for novafurry/novaOS (a fork of malwarepad/cavOS). It's basically just a C application blehh :3

## Preview
<table>
<tbody>
<thead><td>General preview of all current functionality</td></thead>
<tr><td><video src="Readme Assets/NovaOS on 10-08-2025.mp4" controls=yes autoplay=yes>
Updated <time datetime="2025-08-10"><pre>10th August 2025</pre></time>
<small><i>Not Pictured: keyboard input, it just shows the keycode numeral on screen in red text</i></small>
</tbody>
</table>

## Around the codebase
# [build](build)
Bourne-Again SHell script to compile the code. Packs all `.c` files into a single `novaos` executable compiled for `x86_64`
# [gpu.c](gpu.c)
Responsible for all functionality related to drawing to the framebuffer.
