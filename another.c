#include "another.h"
#include <string.h>

void createScript(int X , int Y , int T , double max){
	FILE *file = fopen("plotScript.sh\0" , "w");
	fprintf(file, "i = 0\nn = %d\nset dgrid3d %d,%d\nset hidden3d\nset term gif animate\nset output 'report.gif'\nset xrange [0:%d]\nset yrange [0:%d]\nset zrange[-%f:%f]\nload 'animateGnu'", T , Y, X , X - 1 , Y - 1 , max , max);
	fclose(file);
}

double writeToFile(double* values , int length , int lengthLine , int X , char* place){
	FILE* file = fopen(place , "w");
	char line[100];
	int i , max = 0;
	for(i = 0 ; i < length ; i++){
		int key = (i % lengthLine);
		if(max < values[i])
			max = values[i];
		fprintf(file , "%d %d %f\n" , key  % X , key / X , values[i]);
		if(key + 1 == lengthLine)
			fprintf(file, "\n\n");
	}
	fclose(file);
	return max;
}
