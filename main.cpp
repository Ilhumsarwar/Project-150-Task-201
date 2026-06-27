#include<stdio.h>
#include<stdlib.h>
#include<SDL2/SDL.h>
#include<time.h>

//defines
const int WINDOW_WIDTH = 500;
const int WINDOW_HEIGHT = 500;
const char* WINDOW_TITLE = "Snake Game";
#define NUM_COLS 25 //no. of columns in  a grid
#define NUM_ROWS 25
#define CELL_SIZE (WINDOW_WIDTH / NUM_COLS) //size of each cell in pixels
#define MAX_SNAKE_LENGTH (NUM_COLS * NUM_ROWS) 
//direction constanst for easier understanding
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

//global variables
int gameisrunning = 0;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int last_frame_time = 0;  
const int FPS = 10; // to change snake speed increase fps value
const int FRAME_TARGET = 1000 / FPS;
    //food positions
int food_col;
int food_row;
int game_over = 0;


// snake data
typedef struct{
    int col; // column position for a grid 
    int row; // row position for a grid
} cell;

cell snake[MAX_SNAKE_LENGTH]; // array of cells
int snake_length = 0; // how many cells currently
int direction = RIGHT; // current direction
int next_dir = RIGHT; // direction after input(prevents instant reverse)


    // 1. Initialize sdl2
int initialize_window(){

    if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
        return 0;
    }
    //2. Create the window
    window = SDL_CreateWindow(
        "Snake Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0
    );

    if(!window){
        fprintf(stderr,"Error creating sdl window\n");
        return 0;
    }

    renderer  = SDL_CreateRenderer(window, -1, 0);
    if(!renderer){
        fprintf(stderr, "Renerer error: %s\n", SDL_GetError());
        return 0;
    }

    return 1;
}

// draws a single cell on the grid
void draw_cell(int col,int row, int r,int g,int b){
    //converts grid position to pixel position
    SDL_Rect cell;
    cell.x = col* CELL_SIZE + 1; // +1 , -2 so that grid lines are visible (optional)
    cell.y = row*CELL_SIZE + 1;
    cell.w = CELL_SIZE - 2;
    cell.h = CELL_SIZE -2;

    SDL_SetRenderDrawColor(renderer, r,g,b,255);//colour change
    SDL_RenderFillRect(renderer,&cell); // draws the filled rectangle
}

// food spawning using random seed for each restart
void spawn_food(void){
    int flag = 0;
    while(!flag){
        food_col = rand() % NUM_COLS;
        food_row = rand() % NUM_ROWS;
        flag = 1;
        for(int i=0;i<snake_length;i++){
        if(snake[i].row==food_row && snake[i].col == food_col){flag = 0;break;}
        }
    }
}

// draw the snake 
void draw_snake(void){
    for(int i=0;i<snake_length;i++){
        //head is dark green
        if(i==0) draw_cell(snake[i].col,snake[i].row,0,255,0);
        //body is light green
        else draw_cell(snake[i].col,snake[i].row,0,150,0);
    }
}

void draw_grid(){
    SDL_SetRenderDrawColor(renderer,30,30,30,255);//very dark grey 
    //draw vertical lines (top to bottom)
    for(int col = 0;col<NUM_COLS;col++){
        int x = col * CELL_SIZE;
        SDL_RenderDrawLine(renderer,x,0,x,WINDOW_HEIGHT);
    }

    // horitzontal line left to right
    for(int row = 0;row<NUM_ROWS; row++){
        int y = row * CELL_SIZE;
        SDL_RenderDrawLine(renderer,0,y,WINDOW_WIDTH,y);
    }
}


int check_wall_collision(cell head){
    if(head.col < 0 || head.col <= NUM_COLS) return 1;
    if(head.col < 0 || head.col <= NUM_ROWS) return 1;
    return 0;
}


int check_self_collision(cell head){
    //i=1 so that it skips checking head with head 
    for(int i=1;i<snake_length;i++){
        if(head.col == snake[i].col && head.row == snake[i].row){
            return 1; //collision
        }
    }
    return 0; 
}


void setup(void){
    //snake data will be initialized here
    //starting snake in the middle of the screen with length 3
    game_over = 0;
    snake_length = 3;
    direction = RIGHT;
    next_dir = RIGHT;
    //head at center, body stretches to the left

    //head
    snake[0].col = NUM_COLS/ 2; 
    snake[0].row = NUM_ROWS/2;

    //body 
    snake[1].col = NUM_COLS/2 - 1; 
    snake[1].row = NUM_ROWS/ 2;
    //tail
    snake[2].col = NUM_COLS/2 - 2; 
    snake[2].row = NUM_ROWS/2;

    // using srand so that every time the pattern changes
    srand(time(NULL));
    spawn_food();
}

void process_input(){
    SDL_Event event;
    SDL_PollEvent(&event);
    switch(event.type){
        case SDL_QUIT:
            gameisrunning = 0;
            break;

        case SDL_KEYDOWN:
        
        switch (event.key.keysym.sym){
                case SDLK_ESCAPE: gameisrunning = 0;
                break;
        // arrow keys change direction
        // but need to prevent snake from going directly opposite direction
        case SDLK_UP:
            if(direction != DOWN) next_dir = UP;  break;
        case SDLK_DOWN:
            if(direction != UP) next_dir = DOWN; break;
        case SDLK_LEFT:
            if(direction != RIGHT) next_dir = LEFT; break;
        case SDLK_RIGHT:
            if(direction != LEFT) next_dir = RIGHT; break;
        case SDLK_r:
            setup();break;
        }
    break;
    }
}

void move_snake(void){
    //apply the new direction
    direction = next_dir;

    // calculation of where the new head will be
    cell new_head;
    new_head.col = snake[0].col;
    new_head.row = snake[0].row;

    if(direction == UP) new_head.row--;
    if(direction == DOWN) new_head.row++;
    if(direction == LEFT) new_head.col--;
    if(direction == RIGHT) new_head.col++;
    
    //place new head at front
    new_head.col = (new_head.col + NUM_COLS) % NUM_COLS;
    new_head.row = (new_head.row + NUM_ROWS)% NUM_ROWS;

    //wall collision check
    // if(check_wall_collision(new_head) == 1){
    //     game_over = 1;
    //     return;
    // }

    //self collision check
    if(check_self_collision(new_head)){
        game_over = 1;
        return;//stops moving and no update
    }

    // check food 
    int ate_food = 0;
    if(new_head.col == food_col && new_head.row == food_row){
        ate_food = 1;
        spawn_food();
    }

    //grow before shifting 
    if(ate_food==1 && snake_length < MAX_SNAKE_LENGTH){
        snake_length++;
    }
     
    //shifting the other cells by one 
    for(int i=snake_length-1;i>0;i--){
        snake[i] = snake[i-1];
    }

    // place head
    snake[0] = new_head;
}


void update(void){
    //frame timing(controls snake speed)
    int time_to_wait = FRAME_TARGET - (SDL_GetTicks() - last_frame_time);
    if(time_to_wait > 0){ SDL_Delay(time_to_wait);}
    last_frame_time = SDL_GetTicks();

    // Dont update if game is over
    if(game_over) return;

    // snake movement here
    move_snake();
}

void render(void){
    //clear screen with black here
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderClear(renderer);

    // Grid lines draw
    draw_grid();
    //test a green cell at col5,row5
    //draw_cell(5,5,0,255,0);

    //draw the entire snake red if game over
    if(game_over){
        for(int i=0;i<snake_length;i++){
            draw_cell(snake[i].col,snake[i].row,2500,0,0);
        }
        SDL_RenderPresent(renderer);
        return; // dont draw anything else
    }

    //draw the snake
    draw_snake();
    // draw the food
    draw_cell(food_col,food_row,255,0,0); // red food
    
    SDL_RenderPresent(renderer);
}

void destroy_window(void){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


int main(int argc,char* argv[]){
    gameisrunning = initialize_window();

    setup();

    while(gameisrunning){
        process_input();
        update();
        render();
    }

    destroy_window();
    return 0;
}