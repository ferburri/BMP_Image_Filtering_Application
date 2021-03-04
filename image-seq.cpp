/* 	
	Sequential Programm for a BMP file processing "image-seq.cpp"

	Authors: Fernando Bonor López, Fernando Burrieza Galán, Alcia Madrid Fernández, Alejandro Puch Marcos
	Since: 06/12/2020
	Version: 2.5

*/

// C++17  Libraries
#include <iostream>
#include <filesystem>
#include <math.h>
#include <stdbool.h>

// Working namespaces.
using namespace std;
namespace fs = std::filesystem;
using namespace fs;
using namespace std::chrono;

// Structure to declare a pixel, that contains each of the RGB values.
struct Pixel {
	unsigned char blue;
	unsigned char green;
	unsigned char red;
};

// Structure to declare the additions on different operations, that contains each of the RGB values.
struct Sum {
	int blue;
	int green;
	int red;
};

// --------------------------- CONSTANT VARIABLES ---------------------------
// String variables.
string copyWord = "copy";
string gauss = "gauss";
string sobel = "sobel";

//Declare parameters for Gauss matrix.
int m[5][5] = {{1, 4, 7, 4, 1}, 
				{4, 16, 26, 16, 4}, 
				{7, 26, 41, 26, 7}, 
				{4, 16, 26, 16, 4}, 
				{1, 4, 7, 4, 1}};
int w_gauss = 273;

// Declare parameters for Sobel matrix.
int mx[3][3] = {{1, 2, 1}, 
				{0, 0, 0}, 
				{-1, -2, -1}};
int my[3][3] = {{-1, 0, 1}, 
				{-2, 0, 2}, 
				{-1, 0, 1}};
int w_sobel = 8;

// --------------------------- FUNCTIONS ---------------------------
// Functions to show the default error messages.
int messageInitial(char *argv[]) {
	cout << "Input path: " << argv[2] << endl;
	cout << "Output path: " << argv[3] << endl;
	return -1;
}
int messageFinal() {
	cout << "  ./image-seq operation in_path out_path" << endl;
	cout << "  operation: copy, gauss, sobel" << endl;
	return -1;
}

// Function to check errors in the introduction of the commands.

// This function takes command input and checks if the number of arguments is the expected ( exactly 4), if it isn't it will show an error message indicating that wrong format was used.
// Morever, in case the number of arguments is fine, it will check if the second argument (the one regarding the operation) has an accepted value ( copyWord,gauus or sobel). In case it isn't
// an error message saying that it is a wrong operation will be shown and return.
// The function will return 0 if everything is right or -1 if there is any error. 


int checkCommands (int argc, char *argv[]) {
	
	// Check the number of arguments introduced.
	if (argc != 4) {
		// Error messages.
		cerr << "Wrong format:" << endl;
		return -1;
	}

	// Check that the operation introduced is right.
	else if ((argv[1] != copyWord) && (argv[1] != gauss) && (argv[1] != sobel)) {
		// Error messages.
		cerr << "Unexpected operation:" << argv[1] << endl;
		return -1;
	}
	return 0;
}

// Function to check the control variables of the input and output directories.


int checkControlVaribles (char *argv[], int checkInput, int checkOutput) {
	if (checkInput == 0 || checkOutput == 0) {
		//Error messages.
		messageInitial(argv);
		// Input directory error.
		if (checkInput == 0) {
			cerr << "Cannot open directory [" << argv[2] << "]" << endl;
		}
		// Output directory error.
		else {
			cerr << "Output directory [" << argv[3] << "]" << " does not exist" << endl;
		}
	}
	return -1;
}

// Function to check if the directories exist and have permissions.

int checkDirectory (char *argv[], path directory, path indir, path outdir, int checkInput, int checkOutput) {
	// Check each element of the current directory.
	for(const auto &entry : directory_iterator(directory)) {
		// Modify control variables if the paths match. 
		if (entry == indir) {
			checkInput = 1;
			// Get the permissions of the folder.
			perms p = status(argv[2]).permissions();
			// Check if the input is a directory and check the permissions it has.
			if ((!is_directory(indir)) || (p & perms::others_exec) == perms::none) {
				//Error messages.
				messageInitial(argv);
				cerr << "Cannot open directory [" << argv[2] << "]" << endl;
				return -1;
			}		
		}
		if (entry == outdir) checkOutput = 1;
	}
	if (checkInput == 0 || checkOutput == 0) return checkControlVaribles(argv, checkInput, checkOutput);
	else return 0;
}

// Function to check if an image has BMP format.
// Function will get different properties of the file whose values, in case it is a correct BMP file, will be at the headear of the image. It will check that number of planes is 1, point size is 24 and that compression
// is 0


int checkBMP(FILE *img, char *argv[]) {

	// Get the number of planes of the image. According to BMP deffinition, this field is located at the 26-27th bits of the header.
	fseek(img, 26, SEEK_SET);
	int planes = fgetc(img);

	// Get the point size of the image. According to BMP deffinition, this field is located at the 28-29th bits of the header.
	fseek(img, 28, SEEK_SET);
	int pointSize = fgetc(img);
	
	// Get the compression of the image. According to BMP deffinition, this field is located at the 30-31-32-33rd bits of the header.
	fseek(img, 30, SEEK_SET);
	int compression = fgetc(img);
	
	// Check if the image is valid by checking if all fields have the definition values.
	if(planes == 1 && pointSize == 24 && compression == 0) return 1;
	
	// Error messages if the data does not match.
	messageInitial(argv);
	
	// Also specific errors according to what is wrong.

	// Planes error.
	if (planes != 1) cerr << "Planes is not 1" << endl;

	// Point size error.
	else if (pointSize != 24) cerr << "Bit count is not 24" << endl;

	// Compression error.
	else cerr << "Compression is not 0" << endl;
	
	messageFinal();
	return -1;
}

// Function to obtain the output format of the path

std::string obtainPath(path file){
    //Obtain the filename
    std::string filename = file.filename();
    //Obtain the directory
    std::string dir = file.parent_path();
    std::size_t found = dir.find_last_of("/");
    dir = dir.substr(found+1,dir.length());
    dir.append("/");
    dir.append(filename);
    return dir;
}

// Function to transform either the data array into the data matrix or viceversa.

void arrayMatrix (unsigned char* data, Pixel **matrix, int operation, int height, int width, int totalWidth) {

	// In case operation is equal to 1, the function will transform the arry into matrix.
	if (operation == 1) {
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				matrix[i][j].blue = data[i*totalWidth+3*j];
				matrix[i][j].green = data[i*totalWidth+3*j+1];
				matrix[i][j].red = data[i*totalWidth+3*j+2];
			}
		}
	}
	// In case operation is equal to 0, the function will transform the matrix into array.
	else {
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				data[i*totalWidth+3*j] = matrix[i][j].blue;
				data[i*totalWidth+3*j+1] = matrix[i][j].green;
				data[i*totalWidth+3*j+2] = matrix[i][j].red;		
			}
		}
	}
}

// Function to create a BMP file to store in the outdir directory after doing gauss or sobel.

void createBMPFile(FILE* outputImage, int width, int height) {

	// Initialize the header that has a size of 54 bits.
	unsigned char* header = (unsigned char*)(calloc(54, sizeof(char)));

	// Declare the size of a byte.
    short sizeByte = 256;

	// Now, each of the values of the header will be asigned

	// First two positions of the header.
    header[0] = 'B';
    header[1] = 'M';

	// The total size of the image will be the number of Pixels values plus plus the header size (54) 
    int size = 3 * width * height + 54;

    // The total size is divided into the next four positions of the header.
    header[2] = size % sizeByte;
    header[3] = size / sizeByte;
    header[4] = size / (pow(sizeByte, 2));
    header[5] = size / (pow(sizeByte, 3));

    // The start of the image data will be after the header.
    header[10] = 54;

    // The bitmap header is always 40.
    header[14] = 40;

    // The width of the image is divided into four positions of the header.
    header[18] = width % sizeByte;
    header[19] = width / sizeByte;
    header[20] = width / (pow(sizeByte, 2));
    header[21] = width / (pow(sizeByte, 3));

    // The height is also divided in four positions.
    header[22] = height % sizeByte;
    header[23] = height / sizeByte;
    header[24] = height / (pow(sizeByte, 2));
    header[25] = height / (pow(sizeByte, 3));

    // BMP images have only one plane.
    header[26] = 1;

    // BMP images have 24 point size.
    header[28] = 24;

    // The size of the image data is only the ammount of pizel values.
    size = 3 * width * height;

	// The size is divided into the next four positions of the header.
    header[34] = size % sizeByte;
    header[35] = size / sizeByte;
    header[36] = size / (pow(sizeByte, 2));
    header[37] = size / (pow(sizeByte, 3));

    // The horizontal resolution of an BMP image must be 2835.
	header[38] = 19;
    header[39] = 11;

    // The vertical resolution of an BMP image is also 2835.
	header[42] = 19;
    header[43] = 11;

    // Write the header into the image file.
	fwrite(header, sizeof(char), 54, outputImage);
}

// Function to print the times in the copy event.
void printCopy(std::string file, std::chrono::microseconds loadTime, std::chrono::microseconds storeTime, std::chrono::microseconds totalTime){
	cout << "File: " << '"' << file << '"' << "(time: " << totalTime.count() << ")" << endl;
	cout << "  Load time: " << loadTime.count() << endl;	
	cout << "  Store time: " << storeTime.count() << endl;
}

// Function to print the times in the gauss and sobel events, depending on the decision variable.
void printGaussSobel(std::string file, std::chrono::microseconds loadTime, std::chrono::microseconds storeTime, std::chrono::microseconds gaussTime, std::chrono::microseconds sobelTime, std::chrono::microseconds totalTime, bool decision){
	cout << "File: " << '"' << file << '"' << "(time: " << totalTime.count() << ")" << endl;
	cout << "  Load time: " << loadTime.count() << endl;	
	
	cout << "  Gauss time: " << gaussTime.count() << endl;
	if (!decision) cout << "  Sobel time: " << sobelTime.count() << endl;

	cout << "  Store time: " << storeTime.count() << endl;
}

// Copy function. This function will make a copy of BMP file from indir path into outdir path.

void copyFunction (char *argv[], path indir, path outdir) {
	
	// Check each element of the input directory.
	for(const auto &entry : directory_iterator(indir)) {	
		// Obtain the name of the image file.
		const char *file = entry.path().c_str();
		// Access to clock in order to measure the execution time at the beginning of the programm.
		auto t1 = high_resolution_clock::now();
		// Open the file.
		FILE* img = fopen(file, "r");
		// Acces to the clock to calculate the load time.
		auto t2 = high_resolution_clock::now();
		auto lt = duration_cast<microseconds>(t2-t1);

		// Check if the attributes of the BMP format are right.
		if (checkBMP(img, argv) == 1) {
			// Select the copy options.
			const auto copyOptions = fs::copy_options::update_existing | fs::copy_options::recursive;
			// Access to clock in order to measure the execution time at the beginning of the programm.
			t1 = high_resolution_clock::now();
			// Copy the images from the input directory to the output.
			fs::copy(entry, outdir, copyOptions);
			// Acces to the clock to calculate the store time.
			t2 = high_resolution_clock::now();
			auto st = duration_cast<microseconds>(t2-t1);
			cout << "Time = " << st.count() << " microseconds" << endl;

			// Function to get the output for the message 
			std::chrono::duration<long, std::micro> loadTime = lt;
			std::chrono::duration<long, std::micro> storeTime = st;
			std::chrono::duration<long, std::micro> totalTime = lt + st;
			std::string dir = obtainPath(entry.path());
			printCopy(dir, loadTime, storeTime, totalTime);
		}
		fclose(img);
	}
}

// Function to generate the gaussian operation.
void gaussOperation (Pixel **imInitial, Pixel **imFinal, int height, int width) {
	Sum* sum = new Sum();
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			// Initialize the addition to 0, each time a new element of the matrix is analyzed.
			sum->red = 0;
			sum->green = 0;
			sum->blue = 0;
			// Operate with the summations.
			for (int s = -2; s <= 2; s++) {
				for (int t = -2; t <= 2; t++) {
					if(((i+s) >= 0 && (i+s) < height) && ((j+t) >= 0 && (j+t) < width)) {
						// Update the variables.
						sum->red += m[s+2][t+2] * imInitial[(i+s)][j+t].red;
						sum->green += m[s+2][t+2] * imInitial[(i+s)][j+t].green;
						sum->blue += m[s+2][t+2] * imInitial[(i+s)][j+t].blue;
					}
				}
			}

			// Give the new values to the image matrix.
			imFinal[i][j].blue = sum->blue/w_gauss;
			imFinal[i][j].green = sum->green/w_gauss;
			imFinal[i][j].red = sum->red/w_gauss;					
		}
	}
}

// Function to generate the sobel operation.
void sobelOperation (Pixel **imInitial, Pixel **imFinal, int height, int width) {
	Sum* sum_x = new Sum();
	Sum* sum_y = new Sum();
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			// Initialize the addition to 0, each time a new element of the matrix is analyzed.
			sum_x->red = 0;
			sum_x->green = 0;
			sum_x->blue = 0;

			sum_y->red = 0;
			sum_y->green = 0;
			sum_y->blue = 0;
			// Operate with the summations.
			for (int s = -1; s <= 1; s++) {
				for (int t = -1; t <= 1; t++) {
					if(((i+s) >= 0 && (i+s) < height) && ((j+t) >= 0 && (j+t) < width)) {
						// Update the variables.
						sum_x->red += mx[s+1][t+1] * imInitial[(i+s)][j+t].red;
						sum_x->green += mx[s+1][t+1] * imInitial[(i+s)][j+t].green;
						sum_x->blue += mx[s+1][t+1] * imInitial[(i+s)][j+t].blue;
					
						sum_y->red += my[s+1][t+1] * imInitial[(i+s)][j+t].red;
						sum_y->green += my[s+1][t+1] * imInitial[(i+s)][j+t].green;
						sum_y->blue += my[s+1][t+1] * imInitial[(i+s)][j+t].blue;
					}
				}
			}

			// Give the new value to the image matrix.
			imFinal[i][j].blue = abs(sum_x->blue/w_sobel) + abs(sum_y->blue/w_sobel);
			imFinal[i][j].green = abs(sum_x->green/w_sobel) + abs(sum_y->green/w_sobel);
			imFinal[i][j].red = abs(sum_x->red/w_sobel) + abs(sum_y->red/w_sobel);
		}
	}	
}

// This function will open a BMP file, get its image, apply Gauss and Sobel to it and create a new BMP file with the result image obtained.

void doOperation (char *argv[], path indir, path outdir, bool decision) {

	for(const auto &entry : directory_iterator(indir)) {
		// Obtain the name of the image file.
		const char *file = entry.path().c_str();
		// Access to clock in order to measure the execution time at the beginning of the programm.
		auto t1 = high_resolution_clock::now();
		// Open the file.
		FILE* img = fopen(file, "rw");
		// Acces to the clock to calculate the load time.
		auto t2 = high_resolution_clock::now();
		auto lt = duration_cast<microseconds>(t2-t1);

		// Check if the attributes of the BMP format are right.
		if (checkBMP(img, argv) != -1) {	
			// Obtain the starting byte of data.
			fseek(img, 10, SEEK_SET);
    		int start = fgetc(img) + fgetc(img) * 256 + fgetc(img) * pow(256, 2) + fgetc(img) * pow(256, 3);
			// Obtain the width and the height of the image.
			fseek(img, 18, SEEK_SET);
			int width = fgetc(img) + fgetc(img) * 256 + fgetc(img) * pow(256, 2) + fgetc(img) * pow(256, 3);
			fseek(img, 22, SEEK_SET);
			int height = fgetc(img) + fgetc(img) * 256 + fgetc(img) * pow(256, 2) + fgetc(img) * pow(256, 3);

			// Variable storing the number of pixels.
			int numPixels = width * height;

			// Variables to check how many bits of padding we have to add per row.
			int module = (width * 3) % 4;
			int padding;
				
			if (module == 1) padding = 3;
			else if (module == 3) padding = 1;
			else padding = module;
			
			// Variables storing the total width and size including padding.
			int totalWidth = width * 3 + padding;
			int totalSize = totalWidth * height;

			// Create the data array.
			unsigned char* data = (unsigned char*)(calloc(totalSize, sizeof(char)));
				
			// Read the data after the position where it starts.
			fseek(img, start, SEEK_SET);
			size_t test = fread(data, sizeof(Pixel), totalSize, img);
			test += 1; 
			// Create and declare the initial and final matrixes containing the data.
			Pixel **imInitial = new Pixel*[height];
			Pixel **imFinal = new Pixel*[height]; 
			for (int i = 0; i < height; i++){
				imInitial[i] = new Pixel[width];
				imFinal[i] = new Pixel[width];
			}
			
			// Transform the array into matrix.
			arrayMatrix(data, imInitial, 1, height, width, totalWidth);
			
			// Access to clock in order to measure gaussian time.
			t1 = high_resolution_clock::now();
			// Apply Gauss to the matrix.
			gaussOperation (imInitial, imFinal, height, width);
			t2 = high_resolution_clock::now();
			auto g = duration_cast<microseconds>(t2-t1);

			// Apply Sobel to the matrix.
			t1 = high_resolution_clock::now();
			if (!decision) sobelOperation(imInitial, imFinal, height, width);
			// Acces to the clock to calculate the gauss/sobel time.
			t2 = high_resolution_clock::now();
			auto s = duration_cast<microseconds>(t2-t1);
			
			// Transform the matrix into array.
			arrayMatrix(data, imFinal, 2, height, width, totalWidth);
			
			// Obtain the output file.
			path outputPath = outdir / entry.path().filename();
			const char *outputFile = outputPath.c_str();
			FILE* outputImage = fopen(outputFile, "w");
			// Create the header of the file.
			createBMPFile(outputImage, width, height);
			// Move to byte 54 to start writing.
			fseek(outputImage, 54, SEEK_SET);

			// Access to clock in order to measure the execution time at the beginning of the programm
			t1 = high_resolution_clock::now();
			fwrite(data, sizeof(char), numPixels*3, outputImage);
			// Acces to the clock to calculate the store time.
			t2 = high_resolution_clock::now();
			auto st = duration_cast<microseconds>(t2-t1);
			fclose(outputImage);

			// Function to get the output for the message 
			std::chrono::duration<long, std::micro> loadTime = lt;
			std::chrono::duration<long, std::micro> storeTime = st;
			std::chrono::duration<long, std::micro> gaussTime = g;
			std::chrono::duration<long, std::micro> sobelTime = s;
			std::chrono::duration<long, std::micro> totalTime;
			if (decision) totalTime= lt + st + g;
			else totalTime= lt + st + g + s;
			std::string dir = obtainPath(entry.path());
			printGaussSobel(dir, loadTime, storeTime, gaussTime, sobelTime, totalTime, decision);
		}		
	}
}

// --------------------------- MAIN CODE ---------------------------

// Main code.

int main(int argc, char *argv[]) {
	// Check if the commands are well-introduced.
	if (checkCommands (argc, argv) == -1) {
		return messageFinal();
	}

	// Get the current directory.
	path directory = current_path();

	// Obtain the paths of the input and output directories.
	path indir = directory / argv[2];
	path outdir = directory / argv[3];
	
	// Create control variables for input and output directories.
	int checkInput = 0, checkOutput = 0;

	// Check the directories and the control variables.
	if (checkDirectory (argv, directory, indir, outdir, checkInput, checkOutput) == -1) return messageFinal();

	// If everything is fine, the initial message will be shown in all the cases.
	messageInitial(argv);

	// COPY EVENT.
	if (argv[1] == copyWord) {
		// Copy the images from the input directory to the output directory.
		copyFunction(argv, indir, outdir);
	}

	// GAUSS EVENT.
	else if (argv[1] == gauss) {
		doOperation(argv, indir, outdir, true);
	}
	
	// SOBEL EVENT
	else {
		doOperation(argv, indir, outdir, false);
	}
}


