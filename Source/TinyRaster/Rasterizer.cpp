/*---------------------------------------------------------------------
*
* Copyright © 2016  Minsi Chen
* E-mail: m.chen@derby.ac.uk
*
* The source is written for the Graphics I and II modules. You are free
* to use and extend the functionality. The code provided here is functional
* however the author does not guarantee its performance.
---------------------------------------------------------------------*/
#include <algorithm>
#include <math.h>
#include <iostream>
#include "Rasterizer.h"


Rasterizer::Rasterizer(void)
{
	mFramebuffer = NULL;
	mScanlineLUT = NULL;
}

void Rasterizer::ClearScanlineLUT()
{
	Scanline *pScanline = mScanlineLUT;

	for (int y = 0; y < mHeight; y++)
	{
		(pScanline + y)->clear();
		(pScanline + y)->shrink_to_fit();
	}
}

unsigned int Rasterizer::ComputeOutCode(const Vector2 & p, const ClipRect& clipRect)
{
	unsigned int CENTRE = 0x0;
	unsigned int LEFT = 0x1;
	unsigned int RIGHT = 0x1 << 1;
	unsigned int BOTTOM = 0x1 << 2;
	unsigned int TOP = 0x1 << 3;
	unsigned int outcode = CENTRE;

	if (p[0] < clipRect.left)
		outcode |= LEFT;
	else if (p[0] >= clipRect.right)
		outcode |= RIGHT;

	if (p[1] < clipRect.bottom)
		outcode |= BOTTOM;
	else if (p[1] >= clipRect.top)
		outcode |= TOP;

	return outcode;
}

bool Rasterizer::ClipLine(const Vertex2d & v1, const Vertex2d & v2, const ClipRect& clipRect, Vector2 & outP1, Vector2 & outP2)
{
	//TODO: EXTRA This is not directly prescribed as an assignment exercise.
	//However, if you want to create an efficient and robust rasteriser, clipping is a usefull addition.
	//The following code is the starting point of the Cohen-Sutherland clipping algorithm.
	//If you complete its implementation, you can test it by calling prior to calling any DrawLine2D .

	const Vector2 p1 = v1.position;
	const Vector2 p2 = v2.position;
	unsigned int outcode1 = ComputeOutCode(p1, clipRect);
	unsigned int outcode2 = ComputeOutCode(p2, clipRect);

	outP1 = p1;
	outP2 = p2;

	bool draw = false;

	return true;
}

void Rasterizer::WriteRGBAToFramebuffer(int x, int y, const Colour4 & colour)
{
	if ((x >= 0 && x <= mWidth - 1) && (y >= 0 && y <= mHeight - 1))
	{
		PixelRGBA *pixel = mFramebuffer->GetBuffer();
		if (mBlendMode == ALPHA_BLEND)	//blending by adding the old color from the buffer to the new color passed in by the DrawPoint function
		{
			float a = colour[3]; //setting alpha value
			Colour4 result;
			Colour4 oldCol = pixel[y*mWidth + x]; //getting the current color value from the buffer
			result[0] = a * colour[0] + (1.0 - a) * oldCol[0];	//changing each color value (r,g,b)
			result[1] = a * colour[1] + (1.0 - a) * oldCol[1];
			result[2] = a * colour[2] + (1.0 - a) * oldCol[2];
			pixel[y*mWidth + x] = result; //assigning to the same pixel the result color after the blending
		}
		else
		{
			pixel[y*mWidth + x] = colour;
		}
	}
}

Rasterizer::Rasterizer(int width, int height)
{
	//Initialise the rasterizer to its initial state
	mFramebuffer = new Framebuffer(width, height);
	mScanlineLUT = new Scanline[height];
	mWidth = width;
	mHeight = height;
	mBGColour.SetVector(0.0, 0.0, 0.0, 1.0);    //default bg colour is black
	mFGColour.SetVector(1.0, 1.0, 1.0, 1.0);    //default fg colour is white

	mGeometryMode = LINE;
	mFillMode = UNFILLED;
	mBlendMode = NO_BLEND;

	SetClipRectangle(0, mWidth, 0, mHeight);
}

Rasterizer::~Rasterizer()
{
	delete mFramebuffer;
	delete[] mScanlineLUT;
}

void Rasterizer::Clear(const Colour4& colour)
{
	PixelRGBA *pixel = mFramebuffer->GetBuffer();

	SetBGColour(colour);

	int size = mWidth*mHeight;

	for (int i = 0; i < size; i++)
	{
		//fill all pixels in the framebuffer with background colour
		*(pixel + i) = mBGColour;
	}
}

void Rasterizer::DrawPoint2D(const Vector2& pt, int size)
{
	int x = pt[0];
	int y = pt[1];

	WriteRGBAToFramebuffer(x, y, mFGColour);
}

void Rasterizer::DrawLine2D(const Vertex2d & v1, const Vertex2d & v2, int thickness)
{
	//The following code is basic Bresenham's line drawing algorithm.
	//The current implementation is only capable of rasterise a line in the first octant, where dy < dx and dy/dx >= 0
	//See if you want to read ahead https://www.cs.helsinki.fi/group/goa/mallinnus/lines/bresenh.html

	//TODO:
	//Ex 1.1 Complete the implementation of Rasterizer::DrawLine2D method. 
	//This method currently consists of a partially implemented Bresenham algorithm.
	//You must extend its implementation so that it is capable of drawing 2D lines with arbitrary gradient(slope).
	//Use Test 1 (Press F1) to test your implementation

	//Ex 1.2 Extend the implementation of Rasterizer::DrawLine2D so that it is capable of drawing lines based on a given thickness.
	//Note: The thickness is passed as an argument int thickness.
	//Use Test 2 (Press F2) to test your implementation

	//Ex 1.3 Extend the implementation of Rasterizer::DrawLine2D so that it is capable of interpolating colour across a line when each end-point has different colours.
	//Note: The variable mFillMode indicates if the fill mode is set to INTERPOLATED_FILL. 
	//The colour of each point should be linearly interpolated using the colours of v1 and v2.
	//Use Test 2 (Press F2) to test your implementation

	Vector2 pt1 = v1.position;
	Vector2 pt2 = v2.position;

	//setting all the booleans to false
	bool swapVertex = false;
	bool negativeSlope = false;
	bool swapXY = false;
	bool oct7 = false; //set to keep track of the special case oct7
	float oldYValue = 0;  //I use it to know when I'm jumping to the next scanline because i want to add just one X value even though I need more than 1 to draw the actual line

	//checking all the booleans to determine in which octant is the line
	if (v1.position[0] > v2.position[0])
	{
		swapVertex = true;
	}

	if (swapVertex)
	{
		pt1 = v2.position;
		pt2 = v1.position;
	}

	if (pt2[1] - pt1[1] < 0)
	{
		negativeSlope = true;
	}

	if (abs(pt2[0] - pt1[0]) < abs(pt2[1] - pt1[1]))
	{
		swapXY = true;
	}

	if (swapVertex && swapXY && negativeSlope) //swapping the points back to their original position in order to handle the special case in oct3 
	{
		pt1 = v1.position;
		pt2 = v2.position;
	}

	if (!swapVertex && swapXY && negativeSlope) //line is in oct7, I change a boolean so that I can handle the single case properly
	{
		oct7 = true;
	}

	//start setting all my values to move the line in octant 1
	int dx = pt2[0] - pt1[0];
	int dy = pt2[1] - pt1[1];

	int epsilon = 0;

	int x = pt1[0];
	int y = pt1[1];
	int ex = pt2[0];

	if (swapXY && !oct7) //swapping first so that oct3 is handle correctly with building a specific case.  
	{					 //Not swapping in case line is in oct7 because we want to reflect it first and then swap x and y
		int c = x;
		x = y;
		y = c;
		dx = pt2[1] - x;
		dy = pt2[0] - y;
		ex = pt2[1];
	}

	if (negativeSlope) //if the slope is negative a reflect it to make it positive
	{
		y = -y;
		dy = -dy;
	}

	if (oct7) // handling separatly special case oct7 
	{
		int c = x;
		x = y;
		y = c;
		dx = -pt2[1] - x;
		dy = pt2[0] - y;
		ex = -pt2[1];
	}

	while (x <= ex)
	{
		Vector2 temp(x, y);

		if (negativeSlope && !oct7) // if oct3 I made sure that I first reflect the line and then swap it
		{
			temp[1] = -temp[1];
		}
		if (swapXY)
		{
			int c = temp[0];
			temp[0] = temp[1];
			temp[1] = c;
		}
		if (oct7) //if oct7, I swap XY first and then I reflect the line 
		{
			temp[1] = -temp[1];
		}

		Colour4 colour = v1.colour;

		SetFGColour(colour);

		int xScan = temp[0];
		if (xScan < 0) //if X value is outside the framebuffer, I set it to 0 if it is a negative value 
		{
			xScan = 0;
		}
		if (xScan > mWidth - 1) //or to the maximum value of the framebuffer in case it exceeds it
		{
			xScan = mWidth - 1;
		}
		if (mFillMode == INTERPOLATED_FILLED) //Checking if the fillmode is set to interpolated_filled
		{
			float distancex = (v2.position[0] - v1.position[0]) * (v2.position[0] - v1.position[0]);
			float distancey = (v2.position[1] - v1.position[1]) * (v2.position[1] - v1.position[1]);
			float distanceTotal = sqrt(abs(distancex) + abs(distancey)); // I calculate the total lenght of the line from P1 to P2 

			distancex = (temp[0] - v1.position[0]) * (temp[0] - v1.position[0]);
			distancey = (temp[1] - v1.position[1]) * (temp[1] - v1.position[1]);
			float distanceP = sqrt(abs(distancex) + abs(distancey)); // I calculate the distance from P1 and the point P
			float t = distanceP / distanceTotal; // I scale the value to scale 0 to 1 by doing > totalDistance : pointDistance = 1 : x <

			colour[0] = v1.colour[0] + (v2.colour[0] - v1.colour[0]) *t;
			colour[1] = v1.colour[1] + (v2.colour[1] - v1.colour[1]) *t;
			colour[2] = v1.colour[2] + (v2.colour[2] - v1.colour[2]) *t;

			SetFGColour(colour); 
		}
		if (mGeometryMode == POLYGON && abs(temp[1]) != oldYValue)  //Checking if I'm drawing a polygon and also if my Y value has changed from the previous one
		{
			ScanlineLUTItem newitem = { colour, xScan };		
			mScanlineLUT[(int)abs(temp[1])].push_back(newitem);	// adding value to LUT 
			oldYValue = abs(temp[1]); //Updating Y value I'm currently on
		}
		DrawPoint2D(temp);

		if (thickness > 1) //handling thickness greater than 1  
		{
			int negValue = -(thickness / 2); //I start from a negative value cause I want the original point to be in the middle
			if (swapXY || swapVertex) //if I swap the vertex or swap x y I draw the points in X axis
			{
				temp[0] = temp[0] + negValue;
				for (int i = 0; i < thickness; i++)
				{
					temp[0] = temp[0] + 1;
					DrawPoint2D(temp);
				}
			}
			else //else if I don't swap anything I draw points on the Y axis
			{
				temp[1] = temp[1] + negValue;
				for (int i = 0; i < thickness; i++)
				{
					temp[1] = temp[1] + 1;
					DrawPoint2D(temp);
				}
			}
		}

		epsilon += dy;

		if ((epsilon << 1) >= dx)
		{
			y++;
			epsilon -= dx;
		}

		x++;
	}

}

void Rasterizer::DrawUnfilledPolygon2D(const Vertex2d * vertices, int count)
{
	//TODO:
	//Ex 2.1 Implement the Rasterizer::DrawUnfilledPolygon2D method so that it is capable of drawing an unfilled polygon, i.e. only the edges of a polygon are rasterised. 
	//Please note, in order to complete this exercise, you must first complete Ex1.1 since DrawLine2D method is reusable here.
	//Note: The edges of a given polygon can be found by conntecting two adjacent vertices in the vertices array.
	//Use Test 3 (Press F3) to test your solution.
	int i = 0;
	while (i < count - 1)
	{
		DrawLine2D(vertices[i], vertices[i+1]);
		i++;
	}
	DrawLine2D(vertices[0], vertices[count - 1]);
}

bool sortCheck(ScanlineLUTItem firstX, ScanlineLUTItem secondX) // sort function
{
	return (firstX.pos_x < secondX.pos_x);
}

void Rasterizer::ScanlineFillPolygon2D(const Vertex2d * vertices, int count)
{
	//TODO:
	//Ex 2.2 Implement the Rasterizer::ScanlineFillPolygon2D method method so that it is capable of drawing a solidly filled polygon.
	//Note: You can implement floodfill for this exercise however scanline fill is considered a more efficient and robust solution.
	//      You should be able to reuse DrawUnfilledPolygon2D here.
	//
	//Use Test 4 (Press F4) to test your solution, this is a simple test case as all polygons are convex.
	//Use Test 5 (Press F5) to test your solution, this is a complex test case with one non-convex polygon.

	//clearing the scanline
	ClearScanlineLUT();

	//drawing the polygon outlines and populating LUT
	DrawUnfilledPolygon2D(vertices, count);

	//Calculates Max and Min value of y of the Polygon so that I can avoid scanlining the whole framebuffer everytime
	int yMin = mHeight - 1;
	int yMax = 0;
	for (int i = 0; i < count; i++)
	{
		if (vertices[i].position[1] < yMin)
		{
			yMin = vertices[i].position[1];
			if (yMin < 0) // if the minumum position is less than 0 i set it to 0
			{
				yMin = 0;
			}
		}
		if (vertices[i].position[1] > yMax)
		{
			yMax = vertices[i].position[1];
			if (yMax > mHeight - 1) // if the maximum position exceeds the height i set it to the maximum value
			{
				yMax = mHeight - 1;
			}
		}
	}

	//sorting
	for (int y = yMin; y < mHeight - 1 && y < yMax; y++) //scanlining from the minimum value of Y to the maximum value
	{
		int size = mScanlineLUT[y].size(); // I take the size so I know how many x values I have in the specific scanline[y]
		if (!mScanlineLUT[y].empty() && size >= 2) // if it's not empty and it has more than 1 value
		{
			std::sort(mScanlineLUT[y].begin(), mScanlineLUT[y].end(), sortCheck); // I sort the X values
		}
	}

	//drawing the scanline
	for (int y = yMin; y < mHeight - 1 && y < yMax; y++) //scanlining from the minimum value of Y to the maximum value
	{
		int size = mScanlineLUT[y].size(); // I take the size so I know how many x values I have in the specific scanline[y]
		if (!mScanlineLUT[y].empty() && size > 1) // if it's not empty and it has more than 1 value
		{
			int j = 0;
			if (count % 2 == 0) // check if the polygon has an even number of vertices
			{
				for (j = 0; size >= j + 1; (j = j + 2)) //even number of vertices: I loop through the X values in my scnaline[y] taking pairs to draw the line between them
				{
					Vertex2d firstVert;
					Vertex2d secondVert;
					firstVert.position[0] = mScanlineLUT[y][j].pos_x;
					firstVert.position[1] = y;
					firstVert.colour = mScanlineLUT[y][j].colour;

					secondVert.position[0] = mScanlineLUT[y][j + 1].pos_x;
					secondVert.position[1] = y;
					secondVert.colour = mScanlineLUT[y][j + 1].colour;

					DrawLine2D(firstVert, secondVert);
				}
			}
			else //odd number of vertices: Instead of looping through the X values in my scanline[y] I'm drawing line between the first value in the scanline[y] and the last one 
			{
				j = size - 1; //setting J to the last value in the scanline[y]
				Vertex2d firstVert;
				Vertex2d secondVert;
				firstVert.position[0] = mScanlineLUT[y][0].pos_x;
				firstVert.position[1] = y;
				firstVert.colour = mScanlineLUT[y][0].colour;

				secondVert.position[0] = mScanlineLUT[y][j].pos_x;
				secondVert.position[1] = y;
				secondVert.colour = mScanlineLUT[y][j].colour;

				DrawLine2D(firstVert, secondVert);
			}
		}
	}
	//Ex 2.3 Extend Rasterizer::ScanlineFillPolygon2D method so that it is capable of alpha blending, i.e. draw translucent polygons.
	//Note: The variable mBlendMode indicates if the blend mode is set to alpha blending.
	//To do alpha blending during filling, the new colour of a point should be combined with the existing colour in the framebuffer using the alpha value.
	//Use Test 6 (Press F6) to test your solution
}

void Rasterizer::ScanlineInterpolatedFillPolygon2D(const Vertex2d * vertices, int count)
{
	//TODO:
	//Ex 2.4 Implement Rasterizer::ScanlineInterpolatedFillPolygon
	//2D method so that it is capable of performing interpolated filling.
	//Note: mFillMode is set to INTERPOLATED_FILL
	//      This exercise will be more straightfoward if Ex 1.3 has been implemented in DrawLine2D
	//Use Test 7 to test your solution

	ScanlineFillPolygon2D(vertices, count); //I handle the color interpolation of a polygon in the ScanlineFillPolygon2D
}

void Rasterizer::DrawCircle2D(const Circle2D & inCircle, bool filled)
{
	//TODO:
	//Ex 2.5 Implement Rasterizer::DrawCircle2D method so that it can draw a filled circle.
	//Note: For a simple solution, you can first attempt to draw an unfilled circle in the same way as drawing an unfilled polygon.
	//Use Test 8 to test your solution

	mGeometryMode = POLYGON; // Setting geometry mode to POLYGON so that when drawing the points I will also populate the LUT

	//clearing the LUT
	ClearScanlineLUT();
	
	//drawing circle
	double resultCos = 0;
	double resultSin = 0;
	double r = inCircle.radius; 
	double angle = r/20; // changes the amount of segment based on the lenght of the radius
	double param = angle;
	double centerX = inCircle.centre[0];
	double centerY = inCircle.centre[1];

	while (param <= 360) // loop untill we complete the circle
	{
		//calculating first point
		resultCos = cos(param*PI / 180);
		resultSin = sin(param*PI / 180);
		Vector2 p1;
		p1[0] = centerX + r * resultCos; // x = centerX *r *cos(angle)
		p1[1] = centerY + r * resultSin; // y = centerY *r *sin(angle)
		//calculating second point
		resultCos = cos((param+angle)*PI / 180);
		resultSin = sin((param+angle)*PI / 180);
		Vector2 p2;
		p2[0] = centerX + r * resultCos; 
		p2[1] = centerY + r * resultSin;
		//assigning the points to a Vertex2d
		Vertex2d firstVert;
		Vertex2d secondVert;
		firstVert.position = p1;
		secondVert.position = p2;
		firstVert.colour = inCircle.colour;
		secondVert.colour = inCircle.colour;
		
		DrawLine2D(firstVert, secondVert);
		param = param + angle; 
	}

	//Setting y minimum and maximum value so that I can avoid scanlining the whole framebuffer
	int yMin = inCircle.centre[1] - inCircle.radius;
	int yMax = inCircle.centre[1] + inCircle.radius;

	if (filled == true)
	{
		//sorting
		for (int y = yMin; y < mHeight - 1 && y < yMax; y++)
		{
			int size = mScanlineLUT[y].size();
			if (!mScanlineLUT[y].empty() && size > 1)
			{
				std::sort(mScanlineLUT[y].begin(), mScanlineLUT[y].end(), sortCheck); //sort
			}
		}

		//drawing the scanline
		for (int y = yMin; y < mHeight - 1 && y < yMax; y++)
		{
			int size = mScanlineLUT[y].size();
			if (!mScanlineLUT[y].empty() && size > 1)
			{
				for (int j = 0; size >= j + 1; (j = j + 2))
				{
					Vertex2d firstVert;
					Vertex2d secondVert;
					firstVert.position[0] = mScanlineLUT[y][j].pos_x;
					firstVert.position[1] = y;
					firstVert.colour = mScanlineLUT[y][j].colour;

					secondVert.position[0] = mScanlineLUT[y][size - 1].pos_x;
					secondVert.position[1] = y;
					secondVert.colour = mScanlineLUT[y][size - 1].colour;

					DrawLine2D(firstVert, secondVert);
				}
			}
		}
	}
}

Framebuffer *Rasterizer::GetFrameBuffer() const
{
	return mFramebuffer;
}