# PtDescreen
C implementation of a frequency based image descreen filter

### Goals
The goal of this project is to implement an algorithm similar to the one outlined in [this paper](http://scholarworks.boisestate.edu/cgi/viewcontent.cgi?article=1095&context=electrical_facpubs) (PDF file).
This will probably be implemented as a GIMP plugin, and maybe as a standalone program.
I want the output quality to be on par with (or exceed) commercial descreen software based on similar methods such as Sattva's [Descreen Photoshop plugin](http://www.descreen.net).

### Similar projects
* [GIMP Descreen script](http://registry.gimp.org/node/24411)  
   This is a simple descreen script that works with varying results. It works by performing a Fourier transform on an image using the GIMP [Fourier plugin](http://registry.gimp.org/node/19596), then thresholding the image to find peaks. It masks those peaks with a neutral color and inverses the transform, which results in a descreened image.
   This method works to some degree, but is not precise and will usually end up producing ringing and adding noise to parts of the image that do not contain a screentone (I believe part of the fault in this lies with the Fourier GIMP plugin, as it does not retain all FFT data when transforming the image, thus it produces a lossy inverse transform).
* [Sattva's Descreen Plugin](http://descreen.net/) (Commercial)  
   This is a commercial plugin for Photoshop that works by applying a low-pass filter on the image slightly below the frequency of the screentone, and then eliminating remaining low frequency peaks that cause additional moiré patterns to appear if not removed.
   This works quite well, but I believe it can be achieved without applying a strict low-pass filter, thus retaining some high frequency detail.
