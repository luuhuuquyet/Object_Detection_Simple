#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <cstring>

using namespace std::chrono;

#define ELAPSED_TIME(MSG, ...) auto start = chrono::steady_clock::now(); __VA_ARGS__ ; auto end = chrono::steady_clock::now(); std::cout << "Elapsed time in milliseconds [" << MSG << "] : " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms\n";

#define WIDTH       800
#define HEIGHT      600

#define LEFT_POS    0
#define RIGHT_POS   1
#define TOP_POS     2
#define BOT_POS     3

#define VISUAL_ENABLE

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    int top;
    int left;
    int height;
    int width;
} OUTPUT;

// Find 4 corners
// Highest top point
// Lowest bottom point
// Nearest left point
// Farest right point
void find_coordinates (char* buffer, Point point, Point* Coord){
    unsigned char curent_pixel = *(buffer +  point.y * WIDTH + point.x );
    int cur_x = point.x;
    int cur_y = point.y;

    if (curent_pixel  == 0){
        return;
    }
    
    if(cur_x > (Coord + RIGHT_POS)->x ){
        (Coord + RIGHT_POS)->x  = cur_x;
        (Coord + RIGHT_POS)->y = cur_y;
    }
    
    if (cur_y < (Coord + TOP_POS)->y ){
        (Coord + TOP_POS)->y = cur_y;
        (Coord + TOP_POS)->x = cur_x;
    }
    
    if (cur_y > (Coord + BOT_POS)->y ){
        (Coord + BOT_POS)->y = cur_y;
        (Coord + BOT_POS)->x = cur_x;
    }

    // Clear checked pixel
    *(buffer +  point.y * WIDTH + point.x ) = 0x0;        
    
    // Move left
    find_coordinates(buffer, {.x = cur_x - 1, .y= cur_y},  Coord);
    // Move right
    find_coordinates(buffer, {.x = cur_x + 1, .y= cur_y},  Coord);
    // Move up
    find_coordinates(buffer, {.x = cur_x, .y= cur_y + 1},  Coord);
    // Move down
    find_coordinates(buffer, {.x = cur_x , .y= cur_y - 1},  Coord);

    // Move top left
    find_coordinates(buffer, {.x = cur_x - 1, .y= cur_y - 1},  Coord);
    // Move top right
    find_coordinates(buffer, {.x = cur_x + 1, .y= cur_y - 1},  Coord);
    // Move left down 
    find_coordinates(buffer, {.x = cur_x - 1, .y= cur_y + 1},  Coord);
    // Move right down
    find_coordinates(buffer, {.x = cur_x  + 1, .y= cur_y + 1},  Coord);
}

OUTPUT prepare_output (Point* Coord){
    OUTPUT ret;
    ret.left = (Coord + LEFT_POS)->x; // Left
    ret.top =  std::min({(Coord + LEFT_POS)->y, (Coord+RIGHT_POS)->y, (Coord+TOP_POS)->y, (Coord+BOT_POS)->y });
    ret.height = std::max({(Coord + LEFT_POS)->y, (Coord+RIGHT_POS)->y, (Coord+TOP_POS)->y, (Coord+BOT_POS)->y }) - ret.top;
    ret.width = std::max({(Coord + LEFT_POS)->x, (Coord+RIGHT_POS)->x, (Coord+TOP_POS)->x, (Coord+BOT_POS)->x }) - ret.left;
    return ret;
}
#ifdef VISUAL_ENABLE
void boundary_object(char* buff, OUTPUT out){
    int x = out.left;
    int y = out.top;
    int width = out.width;
    int height = out.height;
    for (int i = x ; i < x + width; i ++){
        *(buff +  y * WIDTH + i ) = 0xff;
        *(buff +  (y + height) * WIDTH + i ) = 0xff;
    }
    for(int i = y ; i < y + height; i++){
        *(buff +  i * WIDTH + x ) = 0xff;
        *(buff +  i * WIDTH + x + width ) = 0xff;
    }    
}
#endif


int main() {
    const size_t n_images = 10;
    const size_t width = 800, height = 600;
    //open file
    std::ifstream infile("data.bin");

    // get length of file
    infile.seekg(0, std::ios::end);
    std::streamsize length = infile.tellg();
    infile.seekg(0, std::ios::beg);
    char* buffer = new char[n_images * height * width];

    // read whole file
    infile.read(buffer, length);

#ifdef VISUAL_ENABLE
    char *draw;    
    char *visual = new char[n_images * height * width];
    std::memcpy(visual, buffer, length);
    std::ofstream outbin("visual.bin");
#endif
    // Output file
    std::ofstream outfile("output.txt");
    char* buff;
    Point Coord[4];
    // Coord[0] = Left
    // Coord[1] = Right
    // Coord[2] = Top
    // Coord[3] = Bot
    int frame_id = 0;
    OUTPUT converted;
    int object_idx = 0;
    while (frame_id < n_images){
        object_idx = 0;
        outfile << "Output frame :" << frame_id << std::endl;
        buff = buffer + frame_id* width * height;
#ifdef VISUAL_ENABLE
        draw = visual + frame_id* width * height;
#endif
        for (auto i = 0 ; i < 800; i ++){ 
            for(auto j = 0 ; j < 600; j++){
                if (*(buff +  j * width + i ) != 0 ){
                    // Found the left point
                    Coord[LEFT_POS] = Coord[RIGHT_POS] = Coord[TOP_POS] = Coord[BOT_POS] = {.x = i, .y = j};
                    find_coordinates(buff, {.x = i, .y = j},  Coord);
                    converted = prepare_output(Coord);
                    outfile << "Ojbect " << object_idx << " : ";
                    outfile << "( " << converted.top << ", " << converted.left << ", ";
                    outfile << converted.height << ", " << converted.width << " )" << std::endl;
#ifdef VISUAL_ENABLE
                        boundary_object(draw, converted);
#endif
                    object_idx++;    
                }
            }
        } 
        frame_id++;       
    }

#ifdef VISUAL_ENABLE
        outbin.write((char*)&visual[0], length);
#endif


    outfile.close();
    delete[] buffer;
    return 0;
}
