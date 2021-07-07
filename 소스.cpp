#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char* argv[]) {
	int samplingrate;
	FILE *f;
	if(f=fopen("config.txt", "r")) {
		fscanf(f, "%d", &samplingrate);
		fclose(f);
	}
	else {
		f = fopen("config.txt", "w");
		fprintf(f, "384000");
		fclose(f);
		samplingrate = 384000;
	}
	for (int i = 1; i < argc; i++) {
		printf("%s\n", argv[i]);
		FILE *f;
		long hdr[11], smpl, depth, count;
		double  dsmpl, div,  div2,error;
		int cnt, outl, lp, lp2, j;
		void *data;
		char *out;
		short *sdata;
		f = fopen(argv[i], "rb");
		fread(hdr, 4, 11, f);
		smpl = hdr[6];
		div = static_cast<double>(samplingrate) / (static_cast<double>(smpl));
		depth = hdr[8] >> 19;
		count = hdr[10] / depth;
		data = malloc(hdr[10]);
		fread(data, depth, count, f);
		fclose(f);
		outl = static_cast<long>(static_cast<long long>(count) * static_cast<long long>(samplingrate) / static_cast<long long>(smpl));
		out = reinterpret_cast<char*>(malloc(outl));
		cnt = 0;
		sdata = reinterpret_cast<short*>(data);
		error = 0;
		while (cnt < outl) {
			error += static_cast<double>(sdata[static_cast<int>(static_cast<double>(cnt) / div)]) / 16384.;
			lp = static_cast<short>(error);
			error -= static_cast<double>(lp);
			lp += 2;
			for (int i = 0; i < 4; i++) out[cnt++] = ((i < lp)?255:128);
		}
		free(data);
		char *filename = reinterpret_cast<char*>(malloc(strlen(argv[i]) + 10));
		char *tok1 = strtok(argv[i], ".");
		char *tok2 = strtok(NULL, ".");
		sprintf(filename, "%s_PWM.%s", tok1, tok2);
		f = fopen(filename, "wb");
		hdr[0] = 0x46464952;
		hdr[1] = outl + 0x24;
		hdr[2] = 0x45564157;
		hdr[3] = 0x20746D66;
		hdr[4] = 0x00000010;
		hdr[5] = 0x00010001;
		hdr[6] = samplingrate;
		hdr[7] = samplingrate;
		hdr[8] = 0x00080001;
		hdr[9] = 0x61746164;
		hdr[10] = outl;
		fwrite(hdr, 4, 11, f);
		fwrite(out, 1, outl, f);
		fclose(f);
		free(out);
	}
}