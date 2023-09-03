#include <gtk/gtk.h>
#include <cairo.h>

// Defines the colors according to rgb standards
#define COLOUR(a,b) cairo_set_source_rgb(a, b.red, b.green, b.blue)

// Structures for coordinates, color codes and presets
typedef struct {
  int x;
  int y;
} coordinate;

typedef struct {
  double red;
  double green;
  double blue;
}colour_codes;

typedef struct{
  int *p;
  char *config_file;
}preset_packet;

// Columns, Rows, Nodes, Colors, Height, Width, Block Size
int columns=400;
int rows=400;

char DRAW_NONE = 0,
     DRAW_WALL = 1,
     DRAW_STRT = 4,
     DRAW_END = 8;

coordinate strt_node={3,3}, end_node={17,17};
colour_codes NONE_CLR ={1.00, 1.00, 1.00},// white
             STRT_CLR ={0.11, 0.88, 0.00},// green
             END_CLR  ={1.00, 0.00, 0.00},// red
             WALL_CLR ={0.00, 0.00, 0.00},
             BRDR_CLR ={0.11, 0.11, 0.11};// grey

int BLK_sz = 25;
int width = 1500, height = 1555;
int btn_pressed=0, node_flag=1;
int terminal_node=1, wall_node=2;

char * config_file="visualizer_config.txt";

// Declaration of functions
static gboolean on_draw_event (GtkWidget *, cairo_t *, gpointer);
static void draw_grid (cairo_t *);
static void draw_nodes (cairo_t *, int(*)[402]);
static void set_grid(gpointer);

// Widgets for window, box and drawing area
GtkWidget *window;
GtkWidget *box1;
GtkWidget *darea;
GtkWidget *tool_bar;
GtkToolButton *stend_btn;
GtkToolButton *wall_button;

// This function is used to draw the entire grid board 
static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data){
  // grid for nodes using preset packet
  int (*node_grid)[402] = (int (*)[402])(((preset_packet *)user_data)->p);

  // get the window settings and dimensions
  GtkWidget *win = gtk_widget_get_toplevel(widget);
  gtk_window_get_size(GTK_WINDOW(win), &width, &height);
  height-=55;

  // Get the block size which is height divided by rows
  BLK_sz = height/rows;
  
  // Draw nodes and grid
  draw_nodes(cr, node_grid);
  draw_grid(cr);

  return FALSE;
}

// This sets the grid up with all the necessary block such as 
// start node, end node, wall blocks, etc
static void set_grid(gpointer user_data){
  int (*node_grid)[402] = (int (*)[402])(((preset_packet *)user_data)->p);

  for(int i=0;i< 402;i++)
    for(int j=0;j< 402;j++)
      node_grid[i][j] = (i==0||j==0 || i==401||j==401)?DRAW_WALL:DRAW_NONE;
      // These are the border points to set the color equal to wall or none

  // Set the start and end node in the grid
  node_grid[strt_node.y][strt_node.x] = DRAW_STRT;
  node_grid[end_node.y][end_node.x] = DRAW_END;
}

// A function used to draw lines for grid
static void draw_grid(cairo_t *cr){
  COLOUR(cr,BRDR_CLR);
  for(int i=0;i<=columns;i++){

    // Set the cursor properly
    cairo_move_to(cr,i*BLK_sz,0);

    // Draw the line according to given dimensions
    cairo_line_to(cr,i*BLK_sz,rows*BLK_sz);
  }

  for(int i=0;i<=rows;i++){
    cairo_move_to(cr,0,i*BLK_sz);
    cairo_line_to(cr,columns*BLK_sz,i*BLK_sz);
  }

  cairo_stroke(cr);
}

// This function is used to draw the nodes onto the screen itself
static void draw_nodes(cairo_t *cr, int node_grid[402][402]){
  for(int i=1;i<=rows;i++){
    for(int j=1;j<=columns;j++){
      // If the grid says to draw nothing
      if(node_grid[i][j]==DRAW_NONE)continue;

      // If the grid has start node, end node or none
      if(i==strt_node.y && j==strt_node.x)COLOUR(cr,STRT_CLR);
      else if(i==end_node.y && j==end_node.x)COLOUR(cr,END_CLR);
      else if(node_grid[i][j]&DRAW_WALL)COLOUR(cr,WALL_CLR);
      else COLOUR(cr,NONE_CLR);

      // Repaint the grid
      cairo_rectangle(cr, (j-1)*BLK_sz, (i-1)*BLK_sz , BLK_sz, BLK_sz);
      cairo_fill(cr);
    }
  }

  cairo_stroke(cr);
}

// Functions to color the nodes when clicked (mouse button)
static gboolean button_up(GtkWidget *widget, GdkEventButton *event, gpointer user_data){
  btn_pressed=0;
}
static gboolean button_down(GtkWidget *widget, GdkEventButton *event, gpointer user_data){
  // Getting grid of nodes
  int (*node_grid)[402] = (int (*)[402])(((preset_packet *)user_data)->p);

  // Getting x and y coordinates of block that was clicked
  int x = ((int)event->x)/BLK_sz+1,
      y = ((int)(event->y))/BLK_sz+1;
  btn_pressed=event->button;

  // Node flag 1 for start and stop button drawing
  if(node_flag==terminal_node){
    // Events to set the start and end node according using 
    // right and left click to set them
    // 1 for left click and 3 for right click
    if(event->button==1 && (end_node.x!=x || end_node.y!=y)){
        strt_node.x=x;
        strt_node.y=y-2;
        node_grid[y-2][x]=DRAW_STRT;
    }else if(event->button==3 && (strt_node.x!=x || strt_node.y!=y)){
        end_node.x=x;
        end_node.y=y-2;
        node_grid[y-2][x]=DRAW_END;
    }

  } else if(node_flag == 2){
      // 2 for drawing walls and stuff
      if((strt_node.x!=x || strt_node.y!=y) && (end_node.x!=x || end_node.y!=y)){
        if(event->button==1)
          node_grid[y-2][x]=DRAW_WALL;
        else if(event->button==3)
          node_grid[y-2][x]=DRAW_NONE;
      }
  }
  
  gtk_widget_queue_draw(widget);
  return TRUE;
}

// A function to toggle the node buttons between wall and start/end node selector
static void toogle_node(GtkToggleToolButton* self, gpointer user_data){
  switch(*(int *)user_data) {
    case 1:{
      // Set terminal node setting and deactivate wall nodes
      if(gtk_toggle_tool_button_get_active((GtkToggleToolButton*)stend_btn)){
        node_flag=terminal_node;
        gtk_toggle_tool_button_set_active((GtkToggleToolButton*)wall_button,FALSE);
      }
      break;
    }
    case 2:{
      // Set wall node setting and deactivate terminal nodes
      if(gtk_toggle_tool_button_get_active((GtkToggleToolButton*)wall_button)){
        node_flag=wall_node;
        gtk_toggle_tool_button_set_active((GtkToggleToolButton*)stend_btn,FALSE);
      }
      break;
    }
  } 
}

int main(int argc, char *argv[]) {
  preset_packet presets;
  presets.config_file=config_file;
  presets.p=malloc(sizeof(int)*402*402);

  gtk_init(&argc, &argv);

  // Set the grid up with nodes
  set_grid(&presets);
  srand(time(0));

  // Making a window widget and setting it up with position, size and label
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), width, height); 
  gtk_window_set_title(GTK_WINDOW(window), "MazeVisualizer");

  // Make a box, drawing area and add them to the window
  box1 = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
  gtk_container_add(GTK_CONTAINER(window), box1);
  
  darea = gtk_drawing_area_new();

  // A tool bar told hold everything
  tool_bar = gtk_toolbar_new();
  gtk_toolbar_set_style(GTK_TOOLBAR(tool_bar), GTK_TOOLBAR_BOTH);

  // Buttons to toggle wall mode or start/end mode
  stend_btn = (GtkToolButton *)gtk_toggle_tool_button_new();
  gtk_tool_button_set_label(stend_btn,"Start/End");
  gtk_toolbar_insert(GTK_TOOLBAR(tool_bar), (GtkToolItem *)stend_btn, -1);

  wall_button = (GtkToolButton *)gtk_toggle_tool_button_new();
  gtk_tool_button_set_label(wall_button,"Wall");
  gtk_toolbar_insert(GTK_TOOLBAR(tool_bar), (GtkToolItem *)wall_button, -1);
  
  gtk_box_pack_start(GTK_BOX(box1),tool_bar,FALSE, FALSE,0);
  gtk_box_pack_start(GTK_BOX(box1),darea,TRUE, TRUE,0);

  // Drawing and exiting function for drawing board and window
  g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), &presets);
  g_signal_connect(window, "button-press-event", G_CALLBACK(button_down), &presets);
  g_signal_connect(window, "button-release-event", G_CALLBACK(button_up), NULL);
  g_signal_connect(G_OBJECT(stend_btn), "toggled",G_CALLBACK(toogle_node), &terminal_node);
  g_signal_connect(G_OBJECT(wall_button), "toggled",G_CALLBACK(toogle_node), &wall_node);
  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

  rows = 40;
  columns = 40;
  set_grid(&presets); 

  gtk_widget_show_all(window);
  gtk_main();

  return 0;
}