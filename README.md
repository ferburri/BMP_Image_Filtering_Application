# BMP_Image_Filtering_Application
The application applies two lters on a set of images in BMP format. The result will be a new image with the same format.

This program is implemented using C++ Language. For every image, one of the filters can be applied:

- Gauss operation
- Sobel operation

Two dierent versions of the program will be developed:

- image-seq (Sequential Version)
- image-par (Parallel Version)

## Execution Mode

The execution command will have 3 parameters:

- Operation
  - copy: No transformation is performed. It only copies files from indir to outdir
  - gauss: Applies the Gaussian blur filter.
  - sobel: Applies the Gaussian blur filter and then the Sobel operator.

- Directory with input images

- Directory output which images will be written.

An example of a good execution could be the following one:
```bash
$image-seq sobel indir outdir
Input path: indir
Output path: outdir
File: "indir/62096.bmp"(time: 73016)
  Load time: 3442
  Gauss time: 38909
```
As you observe, the program will print the execution time of applying the operation.

## How to compile both programs

To compile either image-seq.c or image-par.seq, the command to compile each program is the following one:
```bash
$g++ -std=c++17 -Wall -Wextra -Wno-deprecated -Werror -pedantic -pedantic-errors -o image-seq
```
Optional: if you want to run faster the program, you can enable the **compiler optimizations** with these flags(```-O3 -DNDEBUG```)

### Notes

I provide a input directory with some BMP images, because the format of a BMP image should have specific options. If you have doubts about this, click on this link:
http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
