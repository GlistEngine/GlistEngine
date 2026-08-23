# gSpline Sample

An interactive sample demonstrating the `gSpline` class and its clamped, uniform B-Spline implementation based on De Boor's algorithm.

## Demo

The video below demonstrates control-point dragging, degree adjustment, animation controls, slope visualization and reset functionality.


https://github.com/user-attachments/assets/b44a0b28-37e2-49e6-ac22-7c5318dcf3ce


## Features

* Displays a clamped, uniform B-Spline curve.
* Evaluates the curve using De Boor's algorithm.
* Allows control points to be repositioned interactively.
* Supports changing the spline degree at runtime.
* Displays an animated marker moving along the curve.
* Visualizes the tangent at the animated point.
* Displays the tangent angle in radians.
* Provides pause, resume and reset controls.

## Controls

| Input                | Action                                |
| -------------------- | ------------------------------------- |
| Drag a control point | Reposition the selected control point |
| Up arrow             | Increase the spline degree            |
| Down arrow           | Decrease the spline degree            |
| Space                | Pause or resume the animation         |
| R                    | Reset the sample                      |

## Authors

* Bahar Kucukozer
* Mehmet Sefa Ciftci
