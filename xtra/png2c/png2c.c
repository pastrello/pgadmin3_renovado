/*************************************************************************
 *
 * pgAdmin III - PostgreSQL Tools
 *
 * Copyright (C) 2002 - 2016, The pgAdmin Development Team
 * This software is released under the PostgreSQL Licence
 *
 * png2c - Converts a PNG image into a C vector, and adds accessor
 *         functions that returns a static instances of the image in 
 *         different formats.
 *
 *************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
char *to_upper(const char *istr)
{
	size_t len;
	char *ostr;
	unsigned int x;
	
	len = strlen(istr);
	ostr = (char *)calloc((len * sizeof(char)) + 1, sizeof(char));

	for(x = 0; x < len; x++)
		ostr[x] = toupper(istr[x]);

	ostr[x] = 0;

	return ostr;
}

char *get_filename(const char *istr)
{
	size_t len;
	char *ostr, *p, *c;
	int x = 0;
	
	len = strlen(istr);
	ostr = (char *)calloc((len * sizeof(char)) + 1, sizeof(char));

	/* Get the position of the last path separator */
	p = strrchr(istr, '/');
	if (!p)
		p = strrchr(istr, '\\');

	if (p)
		p++;
	else
		p = (char *)istr;

	/* Loop through the remaining characters and return the filename */

	for (c = p; *c != '\0'; c++)
	{
		ostr[x] = *c;
		x++;
	}
	ostr[x] = 0;

	return ostr;
}

char *clean_name(const char *istr)
{
	size_t len;
	char *ostr;
	unsigned int x;
	
	len = strlen(istr);
	ostr = (char *)calloc((len * sizeof(char)) + 1, sizeof(char));

	for (x = 0; x < len; x++)
	{
		if (!isalnum(istr[x]))
			ostr[x] = '_';
		else
			ostr[x] = istr[x];
	}
	ostr[x] = 0;

	return ostr;
}

/* Reads an integer attribute (e.g. width="16") from the <svg> element */
static int svg_attr(const char *svg, const char *attr)
{
	char key[32];
	const char *start = strstr(svg, "<svg");
	const char *p;

	if (!start)
		return 0;
	snprintf(key, sizeof(key), " %s=\"", attr);
	p = strstr(start, key);
	if (!p)
	{
		snprintf(key, sizeof(key), "\n%s=\"", attr);
		p = strstr(start, key);
	}
	return p ? atoi(p + strlen(key)) : 0;
}

/*
 * SVG input: embed the SVG text and generate the same accessors as for a
 * PNG (name_img, name_bmp, name_ico), rendering the vector icon with
 * wxBitmapBundle::FromSVG at the size declared in the SVG.
 */
static int svg2c(const char *in, const char *out)
{
	FILE *ipf, *opf;
	char *svg, *lname, *uname, *outname;
	long len;
	int width, height, ch, col;

	ipf = fopen(in, "rb");
	if (!ipf)
	{
		fprintf(stderr, "Failed to open input file: %s\n", in);
		return 1;
	}
	fseek(ipf, 0, SEEK_END);
	len = ftell(ipf);
	fseek(ipf, 0, SEEK_SET);
	svg = (char *)calloc(len + 1, 1);
	if (fread(svg, 1, len, ipf) != (size_t)len)
	{
		fprintf(stderr, "Failed to read input file: %s\n", in);
		return 1;
	}
	width = svg_attr(svg, "width");
	height = svg_attr(svg, "height");
	if (width <= 0 || height <= 0)
	{
		fprintf(stderr, "No width/height in the <svg> element of %s\n", in);
		return 1;
	}

	opf = fopen(out, "wb");
	if (!opf)
	{
		fprintf(stderr, "Failed to open output file: %s\n", out);
		return 1;
	}

	/* foo.pngc -> foo_png, so the code keeps using the same names */
	outname = get_filename(out);
	if (strlen(outname) > 1 && outname[strlen(outname) - 1] == 'c')
		outname[strlen(outname) - 1] = 0;
	lname = clean_name(outname);
	uname = to_upper(lname);

	fprintf(opf, "#ifndef %s_H\n", uname);
	fprintf(opf, "#define %s_H\n\n", uname);
	fprintf(opf, "// Generated from %s\n", get_filename(in));
	fprintf(opf, "static const char %s_data[] = {\n", lname);
	col = 1;
	for (long i = 0; i < len; i++)
	{
		if (col >= 48)
		{
			fputc('\n', opf);
			col = 1;
		}
		ch = (unsigned char)svg[i];
		fprintf(opf, "0x%.2x, ", ch);
		col += 6;
	}
	fprintf(opf, "0x00\n};\n\n");

	fprintf(opf, "#include \"wx/bmpbndl.h\"\n\n");
	fprintf(opf, "static wxBitmapBundle %s_bundle()\n", lname);
	fprintf(opf, "{\n");
	fprintf(opf, "	static wxBitmapBundle bnd_%s = wxBitmapBundle::FromSVG(%s_data, wxSize(%d, %d));\n", lname, lname, width, height);
	fprintf(opf, "	return bnd_%s;\n", lname);
	fprintf(opf, "}\n\n");

	fprintf(opf, "static wxImage *%s_img()\n", lname);
	fprintf(opf, "{\n");
	fprintf(opf, "	static wxImage *img_%s = new wxImage();\n", lname);
	fprintf(opf, "	if (!img_%s->IsOk())\n", lname);
	fprintf(opf, "		*img_%s = %s_bundle().GetBitmap(wxSize(%d, %d)).ConvertToImage();\n", lname, lname, width, height);
	fprintf(opf, "	return img_%s;\n", lname);
	fprintf(opf, "}\n");
	fprintf(opf, "#define %s_img %s_img()\n\n", lname, lname);

	fprintf(opf, "static wxBitmap *%s_bmp()\n", lname);
	fprintf(opf, "{\n");
	fprintf(opf, "	static wxBitmap *bmp_%s;\n", lname);
	fprintf(opf, "	if (!bmp_%s || !bmp_%s->IsOk())\n", lname, lname);
	fprintf(opf, "		bmp_%s = new wxBitmap(*%s_img);\n", lname, lname);
	fprintf(opf, "	return bmp_%s;\n", lname);
	fprintf(opf, "}\n");
	fprintf(opf, "#define %s_bmp %s_bmp()\n\n", lname, lname);

	fprintf(opf, "static wxIcon *%s_ico()\n", lname);
	fprintf(opf, "{\n");
	fprintf(opf, "	static wxIcon *ico_%s;\n", lname);
	fprintf(opf, "	if (!ico_%s || !ico_%s->IsOk())\n", lname, lname);
	fprintf(opf, "	{\n");
	fprintf(opf, "		ico_%s = new wxIcon();\n", lname);
	fprintf(opf, "		ico_%s->CopyFromBitmap(*%s_bmp);\n", lname, lname);
	fprintf(opf, "	}\n");
	fprintf(opf, "	return ico_%s;\n", lname);
	fprintf(opf, "}\n");
	fprintf(opf, "#define %s_ico %s_ico()\n\n", lname, lname);

	fprintf(opf, "#endif // %s_H\n", uname);

	fclose(ipf);
	fclose(opf);
	free(svg);
	return 0;
}

int main(int argc, char *argv[])
{
	FILE *ipf, *opf;
	char *lname, *uname;
	int ch, col;

	/* Check the command line */
	if (argc != 3)
	{
		fprintf(stderr, "Usage:\n\n%s <input file> <output file>\n", argv[0]);
		fprintf(stderr, "\nThe input may be a PNG or an SVG file. For an SVG, the variable\n"
		                "names come from the output file (foo.pngc -> foo_png) and the\n"
		                "icon is rendered at the width/height given in the <svg> element.\n");
		exit(1);
	}

	if (strlen(argv[1]) > 4 && strcmp(argv[1] + strlen(argv[1]) - 4, ".svg") == 0)
		return svg2c(argv[1], argv[2]);

	/* Open the input and output files */
	ipf = fopen(argv[1], "rb");
	if (!ipf) 
	{
		fprintf(stderr, "Failed to open input file: %s\n", argv[1]);
		exit(1);
	}

	opf = fopen(argv[2], "wb");
	if (!opf) 
	{
		fprintf(stderr, "Failed to open input file: %s\n", argv[1]);
		exit(1);
	}

	// Get the inclusion guard and variable base names
	lname = clean_name(get_filename(argv[1]));
	uname = to_upper(lname);

	fprintf(opf, "#ifndef %s_H\n", uname);
	fprintf(opf, "#define %s_H\n\n", uname);
	
	fprintf(opf, "static const unsigned char %s_data[] = {\n", lname);

	/* Loop through each input byte and write it to the vector   */
	/* Stick a \n in once we get to a suitable point (8 bytes). */
	col = 1;
	while ((ch = fgetc(ipf)) != EOF) 
	{
		if (col >= 48) 
		{
			fputc('\n', opf);
			col = 1;
		}
		fprintf(opf, "0x%.2x, ", ch);
		col += 6;
 
	}
	fprintf(opf, "\n");
	fprintf(opf, "};\n\n");

	/* wxImage accessor */
	fprintf(opf, "#include \"wx/mstream.h\"\n\n");
	fprintf(opf, "static wxImage *%s_img()\n", lname);
	fprintf(opf, "{\n");
	fprintf(opf, "	if (!wxImage::FindHandler(wxT(\"PNG file\")))\n");
	fprintf(opf, "		wxImage::AddHandler(new wxPNGHandler());\n");
	fprintf(opf, "	static wxImage *img_%s = new wxImage();\n", lname);
	fprintf(opf, "	if (!img_%s || !img_%s->IsOk())\n", lname, lname);
	fprintf(opf, "	{\n");
	fprintf(opf, "		wxMemoryInputStream img_%sIS(%s_data, sizeof(%s_data));\n", lname, lname, lname);
	fprintf(opf, "		img_%s->LoadFile(img_%sIS, wxBITMAP_TYPE_PNG);\n", lname, lname);
	fprintf(opf, "	}\n");
	fprintf(opf, "	return img_%s;\n", lname);
	fprintf(opf, "}\n");
	fprintf(opf, "#define %s_img %s_img()\n\n", lname, lname);

	/* wxBitmap accessor */
	fprintf(opf, "static wxBitmap *%s_bmp()\n", lname);
	fprintf(opf, "{\n");
	fprintf(opf, "	static wxBitmap *bmp_%s;\n", lname);
	fprintf(opf, "	if (!bmp_%s || !bmp_%s->IsOk())\n", lname, lname);
	fprintf(opf, "		bmp_%s = new wxBitmap(*%s_img);\n", lname, lname);
	fprintf(opf, "	return bmp_%s;\n", lname);
	fprintf(opf, "}\n");
	fprintf(opf, "#define %s_bmp %s_bmp()\n\n", lname, lname);

	/* wxIcon accessor */
	fprintf(opf, "static wxIcon *%s_ico()\n", lname);
	fprintf(opf, "{\n");
	fprintf(opf, "	static wxIcon *ico_%s;\n", lname);
	fprintf(opf, "	if (!ico_%s || !ico_%s->IsOk())\n", lname, lname);
	fprintf(opf, "	{\n");
	fprintf(opf, "		ico_%s = new wxIcon();\n", lname);
	fprintf(opf, "		ico_%s->CopyFromBitmap(*%s_bmp);\n", lname, lname);
	fprintf(opf, "	}\n");
	fprintf(opf, "	return ico_%s;\n", lname);
	fprintf(opf, "}\n");
	fprintf(opf, "#define %s_ico %s_ico()\n\n", lname, lname);

	fprintf(opf, "#endif // %s_H\n", uname);
 
	fclose(ipf);
	fclose(opf);
	
	exit(0);
}
