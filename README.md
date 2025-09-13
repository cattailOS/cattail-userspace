# nKernel
[![GitHub license](https://img.shields.io/github/license/novafurry/nKernel)](https://github.com/novafurry/nKernel/blob/master/LICENSE)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/85b7b1d7493544fb8175f73686663979)](https://app.codacy.com/gh/novafurry/nKernel/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![GitHub contributors](https://img.shields.io/github/contributors/novafurry/nKernel)](https://github.com/novafurry/nKernel/graphs/contributors)
[![GitHub commit activity](https://img.shields.io/github/commit-activity/m/novafurry/nKernel)](https://github.com/novafurry/nKernel/commits)
[![Matrix: #novaos:novafurry.win](https://img.shields.io/matrix/novaos%3Anovafurry.win?server_fqdn=matrix.novafurry.win&fetchMode=summary&style=flat&label=Matrix%3A%20%23novaos%3Anovafurry.win&link=https%3A%2F%2Fmatrix.to%2F%23%2F%2523novaos%3Anovafurry.win)](https://matrix.to/#/#novaos:novafurry.win)

cattail's userspace and shell

## Preview
<table>
  <tr>
    <th>Current Functionality</th>
    <th>More to come</th>
    <th> :3 </th>
  </tr>
  <tr>
    <td width="33.33%">
      <video src="https://github.com/user-attachments/assets/2eca7c1f-8ab7-42ab-9d37-712eea88132f" 
          controls></video>
    </td>
    <td width="33.33%"></td>
    <td width="33.33%"></td>
  </tr>
  <tr>
    <td><b>Not Pictured: keyboard input</b><br><small><i>Last Updated: 2025/08/10 <br>(format: ISO8601)</i></small></td>
    <td></td>
    <td></td>
  </tr>
</table>

## Around the codebase
# [build](build)
Bourne-Again SHell script to compile the code. Packs all `.c` files into a single `cattail` executable compiled for `x86_64`
# [gpu.c](gpu.c)
Responsible for all functionality related to drawing to the framebuffer.
# [key.c](key.c)
Handles `evdev` keyboard events. Currently fixed to reading from `/dev/input/event0`
# [mouse.c](mouse.c)
Handles `evdev` PS/2 Mouse events, reading from `/dev/input/mice`
# [wm.c](wm.c)
Manages windows (duh)
# [main.c](main.c)
Sets up all the threads relevant
# [vgafon.c](vgafon.c)
Contains the system font.

