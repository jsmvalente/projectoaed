#include "Defs.h"


struct parkinglot
{
    Graph* g;				     	 	/*Graph that holds the info about the adj blocks*/
    Array graphdecoder; 			    /*Pointer to array of pointers that contain the info about every vertice in the graph*/
    ListNode * accesseshead;            /*List of indexes of the graphdecoder corresponding to an access to reduce the search cost*/
    ListNode * queuehead;
    ListNode * parkedcarshead;
  	int freespots;						/*Number of free parking spots in the park */
};


Graph* GetGraph(ParkingLot* parkinglot)
{
    return (parkinglot->g);
}

/******************************************************************************
 * InitParkingLot
 *
 * Arguments: floors - number of floors (pointers) in the parking lot (array)
 *
 * Returns: pointer to the parking lot
 *
 *****************************************************************************/

ParkingLot * InitParkingLot( FILE * mapconfig, int col, int row, int floors, int accesses )
{
    char *** matrix;
    int *vertices, *ramps;
    ParkingLot * parkinglot;

    vertices = (int*)malloc(sizeof(int));
    VerifyMalloc( (Item) vertices );

    ramps = (int*)malloc(sizeof(int));
    VerifyMalloc( (Item) ramps );

    parkinglot = (ParkingLot*) malloc( sizeof(ParkingLot) );
    VerifyMalloc((Item) parkinglot);
    parkinglot->freespots = 0;
    matrix = MatrixInit(vertices, ramps, mapconfig, col, row, floors); /*Creates string cointaining the map - its a 3d string */
    parkinglot->graphdecoder = GraphDecoderInit(matrix, col, row, floors, *vertices, &(parkinglot->freespots) ); /*Creates array cointaining the Decoder for the graph positions*/
    parkinglot->g = GraphInit(*vertices, matrix, parkinglot->graphdecoder, col, row);
    parkinglot->accesseshead = InitAccesses(accesses, parkinglot->graphdecoder, *vertices);
    parkinglot->parkedcarshead = ListInit();
    parkinglot->queuehead = ListInit();

    /**PrintGraph(GetGraph(parkinglot), *vertices); */ /*prints the graph in the parkinglot */
    FreeMatrix(matrix, col, row);
    free(vertices);
    free(ramps);

    return (parkinglot);
}


/******************************************************************************
 * MatrixInit
 *
 * Arguments: vertices - number or ellements in the graph
 *            mapconfig - pointer to the file containing the map info
 *
 * Returns: matrix - tri-dimensional array containing the map
 *
 *****************************************************************************/
char *** MatrixInit(int * vertices, int * ramps, FILE * mapconfig, int col, int row, int floors)
{
    char *** matrix;
    char string[MAX_STRING];
    int i, a, b, c, x, y;
    int posx, posy, posz;
    char type;

    /* Reset vertices count*/
    (*vertices) = 0;

    /* Reset ramps count*/
    (*ramps) = 0;

    matrix = (char ***) malloc(sizeof(char**) * col); /*Allocates memmory for the 1st dimension of the matrix*/
    VerifyMalloc( (Item) matrix );

    for(i = 0; i < col; i++)
    {
        matrix[i]= (char**) malloc(sizeof(char*) * row);  /*Allocates memmory for the 2nd dimension of the matrix*/
        VerifyMalloc( (Item) matrix[i] );
    }

    for(x = 0; x < col; x++)
    {
        for(y = 0; y < row; y++)
        {
            matrix[x][y]= (char*) malloc(sizeof(char) * floors);  /*Allocates memmory for the 3rd dimension of the matrix*/
            VerifyMalloc( (Item) matrix[x][y] );
        }
    }


    /*Filling in the map*/
    for(c = 0; c < floors; c++)
    {
        for(a = 0; a < row; a++)
        {
            /*Gets line "a" from file into "string"*/
            if(fgets(string, MAX_STRING, mapconfig) == NULL)
            {
                printf("Error reading file.");
                exit(0);
            }
            for(b = 0; b < col; b++)
            {
                /*For each character b (0 to col) in the line a (0 to row)
                 put it in the floormatrix[b][a] meaning line b, collumn a*/
                matrix[b][row-a-1][c] = string[b];  /*  */

                if( string[b] != '@')   /*Counts the number of characteres that are NOT @ (# of vertices in the graph)*/
                    (*vertices)+=1;
                if( string[b] == 'u' || string[b] == 'd')
                    (*ramps)+=1;
            }
        }
        do
        {
            if(fgets(string, MAX_STRING, mapconfig) == NULL)  /*Gets info about accesses */
            {
                printf("Error reading the accesses info from file.");
                exit(0);
            }
            if(string[0] == 'A')
            {
                if(sscanf(string, "%*s  %d %d %d %c\n", &posx, &posy, &posz, &type) != 4)
                {
                    printf("Error reading the accesses info from file string.");
                    exit(0);
                }
                matrix[posx][posy][posz] = type;
            }

        }while(string[0]!='+');
    }

    return (matrix);
}

/******************************************************************************
 * InitAccesses
 *
 * Arguments: acesses - number or ellements in the graph
 *            acessesarray - pointer to array of ints with all the position wit accesses in the graph decoder
 *
 * Returns: -
 *
 *****************************************************************************/

ListNode * InitAccesses(int accesses, Array decoder, int vertices)
{
    int i, *index, counter = 0;
    char type;
  	ListNode* accesseshead = ListInit();

    for(i = 0; i < vertices; i++) /*Goes through every vertice in the decoder to see if it is or not an access */
    {
        type = GetIP_Type(i, decoder);
        if( type == 'R' || type == 'C' || type == 'H' || type == 'E' || type == 'L' ) /*If it is an access */
        {
          	index = (int*) malloc( sizeof(int) );
          	VerifyMalloc((Item) index);

          	*index = i;
            counter++;
            accesseshead = AddNodeToListHead(accesseshead, (Item) index);
        }
    }
    if(counter != accesses)
    {
        printf("Error. Number of read accesses doesnt match info from file.");
        exit(0);
    }

    return accesseshead;
}

/******************************************************************************
 * InitRamps
 *
 * Arguments: ramps - number or ellements in the graph
 *            acessesarray - pointer to array of ints with all the position wit ramps in the graph decoder
 *
 * Returns: -
 *
 *****************************************************************************/

int * InitRamps(int ramps, Array decoder, int vertices)
{
    int i, count=0;
    int * auxarray;
    char type;


    /*Allocates memory for the ramps array*/
    auxarray = (int *) malloc( ramps * sizeof(int) );
    VerifyMalloc((Item) auxarray);

    for(i = 0; i < vertices; i++) /*Goes through every vertice in the decoder to see if it is a ramp */
    {
        type = GetIP_Type(i, decoder);
        if( type == 'u' || type == 'd' ) /*If it is a ramp */
        {
            auxarray[count] = i;
            count++;
        }
    }
    return(auxarray);
}

/******************************************************************************
 * FreeMatrix
 *
 * Arguments: matrix - number or ellements in the graph
 *            col;row;floors - dimensions of the matrix
 *
 * Returns: -
 *
 *****************************************************************************/
void FreeMatrix(char *** matrix, int col, int row)
{
    int x, y;

    for(x=0; x < col; x++)
    {
        for(y=0; y < row; y++)
            free(matrix[x][y]);
    }

    for(x=0; x < col; x++)
        free(matrix[x]);

    free(matrix);
}

int GetVertices(ParkingLot* parkinglot)
{
    int vertices;

    vertices = GetGraphVertices(parkinglot->g);

    return vertices;
}

Array GetDecoder(ParkingLot* parkinglot)
{
    Array graphdecoder;

    graphdecoder = parkinglot->graphdecoder;

    return graphdecoder;
}

ListNode * GetAccesses(ParkingLot* parkinglot)
{
    return parkinglot->accesseshead;
}

int GetFreeSpots(ParkingLot* parkinglot)
{
    return (parkinglot->freespots);
}

void IncFreeSpots(ParkingLot* parkinglot)
{
    (parkinglot->freespots)++;

    return;
}

void DecFreeSpots(ParkingLot* parkinglot)
{
    (parkinglot->freespots)--;

    return;
}

void SetParkedListHead(ParkingLot* parkinglot, ListNode* head)
{
    parkinglot->parkedcarshead = head;
}

ListNode* GetParkedListHead(ParkingLot* parkinglot)
{
    return (parkinglot->parkedcarshead);
}


void SetQueueHead(ParkingLot* parkinglot, ListNode* head)
{
    parkinglot->queuehead = head;
}

ListNode* GetQueueHead(ParkingLot* parkinglot)
{
    return (parkinglot->queuehead);
}

void FreeParkingLot(ParkingLot * parkinglot)
{
  FreeDecoder( GetDecoder( parkinglot ), GetVertices( parkinglot ) );
  FreeGraph( GetGraph( parkinglot ) ) ;
  freeLinkedList( GetAccesses( parkinglot) );
  freeLinkedList( GetQueueHead(parkinglot) );
  FreeParkedCars( GetParkedListHead(parkinglot) );
  free(parkinglot);
}
