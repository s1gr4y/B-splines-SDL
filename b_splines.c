#define SDL_MAIN_HANDLED

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include "SDL/include/SDL2/SDL.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int mousePosX, mousePosY;
int xnew, ynew;
#define N 5	//points
#define K 2	//deg + 1 so 3 == 4 (cubic), 2 == 3 (quad), and 1 == 2  (linear)
float knotList[N+K+1] = {0.0f};	//size of k+n+1 == 4+4+1 == 9 (ex)

float BasisFunc(int i, int deg, float t, float* knots) {
	if (deg == 0 && knots[i] <= t && knots[i+1] > t) {
		return 1.0f;
	} else if (deg == 0) {
		return 0.0f;
	}
	//printf("knot at %d is %f\n", i, knots[i]);
	//need to keep recursing
	return (((t-knots[i])*BasisFunc(i, deg-1, t, knots))/(knots[i+deg] - knots[i])) +
	(((knots[i+deg+1]-t)*BasisFunc(i+1, deg-1, t, knots))/(knots[i+deg+1] - knots[i+1]));
	
}

float Interpolate(int* p, float t, int size, int deg, float* knots) {
	float sum = 0.0f;
	for (int i = 0; i < size; i++) {
		sum += p[i] * BasisFunc(i, deg, t, knots);
		if (t <= 0.0) {
			printf("p[%d] is %d\n", i, p[i]);
			printf("BasisFunc is %d\n", BasisFunc(i, deg, t, knots));
		}
	}
	if (t <= 0.0) {
		printf("sum is %f\n", sum);
	}
	return sum;
}

/*Function to draw all other 7 pixels present at symmetric position*/
void drawCircle(int xc, int yc, int x, int y)
{
	SDL_RenderDrawPoint(renderer,xc+x,yc+y);
	SDL_RenderDrawPoint(renderer,xc-x,yc+y);
	SDL_RenderDrawPoint(renderer,xc+x,yc-y);
	SDL_RenderDrawPoint(renderer,xc-x,yc-y);
	SDL_RenderDrawPoint(renderer,xc+y,yc+x);
	SDL_RenderDrawPoint(renderer,xc-y,yc+x);
	SDL_RenderDrawPoint(renderer,xc+y,yc-x);
	SDL_RenderDrawPoint(renderer,xc-y,yc-x);
}

/*Function for circle-generation using Bresenham's algorithm */
void circleBres(int xc, int yc, int r)
{
	int x = 0, y = r;
	int d = 3 - 2 * r;
	while (y >= x)
	{
		/*for each pixel we will draw all eight pixels */
		drawCircle(xc, yc, x, y);
		x++;

		/*check for decision parameter and correspondingly update d, x, y*/
		if (d > 0)
		{
			y--;
			d = d + 4 * (x - y) + 10;
		}
		else
			d = d + 4 * x + 6;
		drawCircle(xc, yc, x, y);
	}
}

/* Function that take input as Control Point x_coordinates and
Control Point y_coordinates and draw all sub bezier curves */
void BSplines(int x[], int y[])
{
	double xu = 0.0 , yu = 0.0 , u = 0.0;
	int i = 0 ;
	for(u = knotList[K] ; u <= knotList[N] ; u += 0.0001)	//t_(k-1) <= t <= t_(n+1)
	{
		/* //for 4 point bezier curve
		xu = pow(1-u,3)*x[0]+3*u*pow(1-u,2)*x[1]+3*pow(u,2)*(1-u)*x[2]+pow(u,3)*x[3];
		yu = pow(1-u,3)*y[0]+3*u*pow(1-u,2)*y[1]+3*pow(u,2)*(1-u)*y[2]+pow(u,3)*y[3];
		*/
		xu = Interpolate(x, u, N, K, knotList);
		yu = Interpolate(y, u, N, K, knotList);
		SDL_RenderDrawPoint(renderer , (int)xu , (int)yu);
	}
}

int main(int argc, char* argv[]) {
	//init knotList
	for (int i = 0; i <= N+K+1; i++) {	//to make it clamped we need to set beg (K) digits and end (K) digits to 0 and 1.
		knotList[i] = 1.0f*(i/(N+K+1.0f));
		///*	//uncomment for clamped
		if (i < K-1) {
			knotList[i] = 0.0f;
		} else if (i > N+2) {
			knotList[i] = 1.0f;
		} else {
			//k[i] = 1.0f*(i/(N+K+1.0f));
			knotList[i] = 1.0f*((i-K+2)/(float)((N+K+5)-(2*K)));
			printf("val is %d\n", 1+i-K);
		}
		//*/
		//printf("%f\n", knotList[i]);
	}
	/*
			function to create a window and default renderer.
			int SDL_CreateWindowAndRenderer(int width
										,int height
										,Uint32 window_flags
										,SDL_Window** window
										,SDL_Renderer** renderer)
			return 0 on success and -1 on error
		*/
	
	//init sdl
	///*
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
		if(SDL_CreateWindowAndRenderer(640, 480, 0, &window, &renderer) == 0)
		{
			SDL_bool done = SDL_FALSE;

			int i = 0;
			int x[N], y[N], flagDrawn = 0 ;

			while (!done) {
				SDL_Event event;

				//set background color to black
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
				SDL_RenderClear(renderer);

				//set draw color to white
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

				//We are drawing b splines for N control points
				if(i == N) {
					BSplines(x, y);
					flagDrawn = 1;
				}
				//could make this a for loop
				for (int f = 0; f < i; f++) {
					SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
					circleBres(x[f] , y[f] , 8);
					if (f >= 1) {
						SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
						SDL_RenderDrawLine(renderer , x[f-1] , y[f-1] , x[f] , y[f]) ;
					}
				}
				/*
				for (int f = 1; f < i; f++) {
					SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
					SDL_RenderDrawLine(renderer , x[f-1] , y[f-1] , x[f] , y[f]) ;
				}
				*/

				//Polling SDL events
				if (SDL_PollEvent(&event)) {
					//if window cross button clicked then quit from window
					if (event.type == SDL_QUIT) {
						done = SDL_TRUE;
					}
					//Mouse Button is Down
					if(event.type == SDL_MOUSEBUTTONDOWN) {
						//If left mouse button down then store that point as control point
						if(event.button.button == SDL_BUTTON_LEFT) {
							//store only N points
							if(i < N) {
								printf("Control Point(P%d):(%d,%d)\n", i, mousePosX, mousePosY) ;

								//set Mouse x and y to the x and y of point in array
								x[i] = mousePosX;
								y[i] = mousePosY;
								i++;
							}
						}
					}
					//Mouse is in motion
					if(event.type == SDL_MOUSEMOTION) {
						//get x and y positions from motion of mouse
						xnew = event.motion.x ;
						ynew = event.motion.y ;

						int j;

						//change coordinates of control point after bezier curve has been drawn
						if(flagDrawn == 1) {
							for(j = 0 ; j < i; j++) {
								//Check mouse position if in b/w circle then change position of that control point to mouse new position which are coming from mouse motion
								if((float)sqrt(abs(xnew-x[j]) * abs(xnew-x[j])
									+ abs(ynew-y[j]) * abs(ynew-y[j])) < 8.0) {
									//change coordinate of jth control point
									x[j] = xnew ;
									y[j] = ynew ;
									printf("Changed Control Point(P%d):(%d,%d)\n", j, xnew, ynew) ;
								}
							}
						}
						//updating mouse positions to positions coming from motion
						mousePosX = xnew ;
						mousePosY = ynew ;
					}
				}
				//show the window
				SDL_RenderPresent(renderer);
			}
		}
		//Destroy the renderer and window
		if (renderer) {
			SDL_DestroyRenderer(renderer);
		}
		if (window) {
			SDL_DestroyWindow(window);
		}
	}
	//clean up SDL
	SDL_Quit();
	//*/
	printf("Bye!\n");
	return 0;
}
