#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "raylib.h"

// compile command : gcc wordle.c -o wordle.exe -lraylib -lopengl32 -lgdi32 -lwinmm

// Some constant values to be used
#define MAX 20000
#define spacing 10
#define screenWidth 800
#define screenHeight 800
#define buttonW 80
#define buttonH 80

// Some GUI Colors
#define COL_BG (Color){57, 53, 51 , 255}
#define COL_TEXT (Color){75, 112, 70 , 255}

enum game {
    menu,
    start,
    over,
};


enum game gamestate; //Current state of the game for the main game loop

int guesses = 6; //Number of guesses allowed set at 6 for default.
int length = 5; //Length of word set as 5 for default.

int currentLetter = 2; //Current Button selected 0 = 3 letter 1 = 4 letters 2 = 5 letters
int currentGuess = 2; //Current button selected 0 = 4 guesses 1 = 5 guesses 2 = 6 guesses

int row = 0; //The current row of the typed letters by the user (for displaying)
int col = 0; //The current column of the typed letters by the user (for displaying)
int result = 0;

char words[MAX][6]; //List of words to be filled by the fetchwords function
int wordCount = 0; //Total words fetched by the 
int highscore[3] = {99 , 99  , 99}; // Highscores for 3 , 4 and 5 letters.

char guessHistory[6][6]; //All prior guesses entered by the user
int highlightHistory[6][5]; //All highlights to previous guesses by the user

int highlight[5]; // What to highlight each letter as 0 = green 1 = yellow 2 = orange 3 = grey
int letters[26]; // List of how many times an alphabet occurs within the word.

char typed[6]; //String of what has currently been typed by the user
char currentWord[6]; //What word has been selected 
char message[500]; //Any message to display to user goes here


int fetchWord() //Uses the global variable of length to decide which list to fetch the word from and returns it's index.
{
    char filename[10];
    char temp[100];
    
    sprintf(filename , "%d.txt" , length);

    FILE *file = fopen(filename , "r");

    if(file == NULL)
    {
        printf("No text file in folder , please check and try again.");
    }

    while (fscanf(file , "%99s" , temp) == 1)
    {
        for(int i = 0 ; i < length ; i++) //Capitalizing every letter.
        {
            if(temp[i] >= 'a' && temp[i] <= 'z')
            {
                temp[i] -= ('a' - 'A');
            }

            words[wordCount][i] = temp[i];
        }

        words[wordCount][length] = '\0';
        wordCount += 1;
    }

    fclose(file);

    int random = rand() % wordCount;
    return random;
    
}

void countLetters(char choice[length + 1]) //Counting the quantity of each letter A-Z.
{
    int current;

    for(int i = 0 ; i < length ; i++)
    {
        if(choice[i] >= 'a')
        {
            choice[i] -= ('a' - 'A');
        }
        
        current = (int)(choice[i] - 'A'); //Subtracting the ASCII code of A from the letter at index i to get a range from 0 to 25 in ASCII which is the size of the letters array and then typecasting to integer.
        letters[current] += 1;

    }
}

int checkWord(char word[length+1] , char guess[length+1]) //Compares a guess word with the choice and return 0 if not matched and 1 if matched correctly. As well as updates the highlight array.
{
    int correct[26] = {0}; //Number of each letter in correct position.
    int yellow[26] = {0}; //Number of yellows given for each letter
    int green = 0; //Total letters in the correct position for win check.
    char current; 

    for (int i = 0; i < length; i++)
    {
        highlight[i] = -1;
    }

    for(int i = 0 ; i < length ; i++) //Calculates the total number of correct letters.
    {
        current = guess[i];
        if(current == word[i])
        {
            current -= 'A';
            correct[(int)current] += 1;
            green += 1;
        }
    }

    for(int j = 0 ; j < length ; j++) //Uses previous calculations of total correct to highlight.
    {
        current = guess[j];
        if(current == word[j]) //If the letter is in the correct position it will either be green or orange.
        {
            current -= 'A';
            if(correct[current] + yellow[current] == letters[current]) //If the total number of that letter used inside the word are the same as in the guess then it will be green.
            {
                highlight[j] = 0;
            }
            else if(correct[current] + yellow[current] < letters[current]) //If the total of word is higher than what has been used in guess then it will be orange.
            {
                highlight[j] = 2;
                correct[current] -= 1; //Decrement correct count or else there will be extra oranges i.e 2 oranges but only 1 extra of that letter un-guessed.
            }
        }
        else
        {
            current -= 'A';
            if((letters[current] > 0) && (yellow[current] < (letters[current] - correct[current]))) //If the selected letter has been used in the word and the current count of yellows does not exceed the total yellows of that letter remaining i.e total - correct then it will be yellow.
            {
                yellow[current] += 1; //Increment yellow count so that it doesnt exceed maximum possible of that letter i.e 2 yellows but only 1 extra of that letter un-guessed.
                highlight[j] = 1;
            }
            else //If none of the above then grey.
            {
                highlight[j] = 3;
            }
        }
    }

    if(green == length) //If total correct letters == length then all letters are correct therefore win.
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int validWord(char guess[length+1]) // linearly goes through the list of words to see if the guess matches with any word in the list
{
    for(int i = 0 ; i < wordCount ; i++)
    {
        if(strcmp(guess , words[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

void fetchHighscore() //Checks a local flatfile with highscores stored in it and reads them into the highscore array
{
    int current, count;
    char filename[10] = "hs.txt";

    count = 0;

    FILE *file = fopen(filename , "r");

    while(fscanf(file , "%d" , &current) == 1 && count < 3) //Continues till first 3 lines have been read or the eof is reached
    {
        highscore[count] = current;
        count += 1;
    }
    fclose(file);
}

void setHighscore(int score) //Overwrites the old highscore text file with the new highscore array.
{
    highscore[length - 3] = score;
    char filename[10] = "hs.txt";
    char temp[100];

    FILE *file = fopen(filename , "w");

    for(int i = 0 ; i < 3 ; i++)
    {
        sprintf(temp , "%d\n" , highscore[i]);
        fputs(temp , file);
    }
    fclose(file);
}

//-------------------------- GUI FUNCTIONS ---------------------------------

void drawCentered(char text[100] , int size , int y , Color col) //Function to write text centred on the X axis
{
    int textSize = MeasureText(text , size);

    int x = (GetScreenWidth() - textSize) / 2;

    DrawText(text , x , y , size , col);
}

int button(Rectangle rect) //Function to check if the mouse has clicked on a rectangle or not
{
    Vector2 mouse = GetMousePosition();
    bool colliding = CheckCollisionPointRec(mouse , rect);

    if(colliding && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        return 1;
    }
    else
    {
        return 0;
    }

}

void drawMenu() //Function to draw the home screen or menu
{
    drawCentered("WORD GUESSING GAME" , 40 , 100 , COL_TEXT);
    
    drawCentered("LETTERS" , 30 , 200 , COL_TEXT);
    

    int centre = screenWidth/2;
    Rectangle letterButtons[3] = {{centre - (buttonW*1.5 + spacing) , 250 , buttonW , buttonH} , {centre - (buttonW * 0.5) , 250 , buttonW , buttonH} , {centre + buttonW*0.5 + spacing , 250 , buttonW , buttonH}};
    for (int i = 0; i < 3; i++) //Checking length button clicks
    {
        int result = button(letterButtons[i]);
        if(result)
        {
            currentLetter = i;
            length = i + 3;
        }
    }
    
    char label[2];

    Vector2 mouse = GetMousePosition();

    for (int i = 0; i < 3; i++) //drawing length buttons according to click and hover checks
    {
        Color buttonColor = GRAY;
        sprintf(label , "%d\0" , i+3);

        if (currentLetter == i)
        {
            buttonColor = GREEN;
        }
        else if (CheckCollisionPointRec(mouse , letterButtons[i]))
        {
            buttonColor = LIGHTGRAY;
        }
            
        DrawRectangleRec(letterButtons[i], buttonColor);
        int textWidth = (MeasureText(label , 15));
        DrawText(label , letterButtons[i].x + (letterButtons[i].width - textWidth) / 2 , letterButtons[i].y + (letterButtons[i].height - 20) / 2 , 20, WHITE);
            
    }


    drawCentered("GUESSES" , 30 , 350 , COL_TEXT);

    Rectangle guessButtons[3] = {{centre - (buttonW*1.5 + spacing) , 400 , buttonW , buttonH} , {centre - (buttonW * 0.5) , 400 , buttonW , buttonH} , {centre + buttonW*0.5 + spacing , 400 , buttonW , buttonH}};

    for(int i = 0 ; i < 3 ; i++) // checking guess button click
    {
        int result = button(guessButtons[i]);
        if(result)
        {
            currentGuess = i;
            guesses = i + 4;
        }
    }

    for (int i = 0; i < 3; i++) //drawing guess buttons according to click and hover checks
    {
        Color buttonColor = GRAY;
        sprintf(label , "%d\0" , i+4);

        if (currentGuess == i)
        {
            buttonColor = GREEN;
        }
        else if (CheckCollisionPointRec(mouse , guessButtons[i]))
        {
            buttonColor = LIGHTGRAY;
        }
            
        DrawRectangleRec(guessButtons[i], buttonColor);
        int textWidth = (MeasureText(label , 15));
        DrawText(label , guessButtons[i].x + (guessButtons[i].width - textWidth) / 2 , guessButtons[i].y + (guessButtons[i].height - 20) / 2 , 20, WHITE);
            
    }

    int text = MeasureText("PLAY GAME" , 30);
    Rectangle startButton = {centre-((text + 2*spacing)/2) , 500 , text + 2*spacing , 80};
    
    Color play = GRAY;

    if(CheckCollisionPointRec(mouse , startButton))
    {
        play = LIGHTGRAY;
    }

    if(button(startButton))
    {
        //Starting game and resetting all game variables to default values
        gamestate = start;
        memset(letters, 0, sizeof(letters));
        message[0] = '\0';
        strcpy(currentWord , words[fetchWord()]);
        countLetters(currentWord);
        result = 0;
        row = 0;
        col = 0;
        typed[0] = '\0';
        memset(guessHistory, 0, sizeof(guessHistory));
        memset(highlightHistory, 0, sizeof(highlightHistory));
    }

    DrawText("Highscore: " , 10 , 300 , 25 , COL_TEXT);

    for(int i = 0 ; i < 3 ; i++) // Printing list of highscores for each letter.
    {  
        char temp[100];
        if(highscore[i] <= 6 && highscore[i] >= 1) // Checking for valid score.
        {   
            sprintf(temp , "%d letters: %d" , i+3 , highscore[i]);
            DrawText(temp , 15 , 350 + i*40 , 15 , COL_TEXT);
        }
        else
        {
            sprintf(temp , "%d letters: --" , i+3); // Printing blank otherwise.
            DrawText(temp , 15 , 350 + i*40 , 15 , COL_TEXT);
        }
    }

    DrawRectangleRec(startButton , play);

    drawCentered("PLAY GAME" , 30 , 525 , COL_TEXT);

    DrawText("Rules" , 10 , 650 , 20 , RAYWHITE);
    DrawText("letters highlighted green are in the correct position with no extras of that letter undiscovered" , 10 , 675 , 15 , GREEN);
    DrawText("letters highlighted yellow are present in the word but somehwere else" , 10 , 700 , 15 , YELLOW);
    DrawText("letters highlighted orange are in the correct position but extras of that letter exist and are undiscovered" , 10 , 725 , 15 , ORANGE);
    
}

void gameStart()
{
    drawCentered("WORD GUESSING GAME" , 30 , 50 , COL_TEXT);
    
    int centreX = GetScreenWidth()/2; //Calculations to use for drawing the grid
    int centreY = GetScreenHeight()/2;
    
    int totalW = ((buttonW + spacing) * length);
    int totalH = ((buttonH + spacing) * guesses);

    int startW = centreX - totalW/2;
    int startH = centreY - totalH/2;

    int averageW = totalW/length;
    int averageH = totalH/guesses;

    int posX[length];
    int posY[guesses];

    for(int i = 0 ; i < length ; i++) //Calculating the X coordinates of each box in a row
    {
        posX[i] = startW + (averageW * i);
    }

    for(int i = 0 ; i < guesses ; i++)//Calculating the Y coordinates of each box in a column
    {
        posY[i] = startH + (averageH * i);
    }

    for(int i = 0 ; i < length ; i++) //Drawing the grid of boxes depending on total guesses and max length
    {
        for(int j = 0 ; j < guesses ; j++)
        {
            DrawRectangle(posX[i] , posY[j] , buttonW , buttonH , LIGHTGRAY);
        }
    }

    int key = GetCharPressed(); //Reading player input

    while(key > 0 && row < guesses && gamestate != over) //While there is an input, max guesses havent been reached and game hasnt ended
    {
        if(key >= 'a' && key <= 'z') //Converting input to capital
        {
            key -= ('a' - 'A');
        }
        
        if(key >= 'A' && key <= 'Z' && col < length) //Checking if max length of input has been taken and if it is a character
        {
            typed[col] = (char)key;
            col += 1;
            typed[col] = '\0';
        }
        key = GetCharPressed(); //Reading input again to keep the loop going
        
    }

    if(IsKeyPressed(KEY_BACKSPACE) && col > 0) //Removing most recent character read if backspace is presses
    {
        col -= 1;
        typed[col] = '\0';
    }

    if(IsKeyPressed(KEY_ENTER) && col == length && row < guesses) //If enter is pressed checks if the length of the typed word is corred and if max guesses hasnt been reached
    {
        int valid = validWord(typed); //Checks if its a valid word
        if(valid)
        {
            result = checkWord(currentWord, typed); //Highlight calculations and result checking
            for(int i = 0 ; i < length ; i++) //Adding to highlight history to display previous guesses correctly
            {
                highlightHistory[row][i] = highlight[i];
            }
            strcpy(guessHistory[row] , typed); //Copying current guess to history
            col = 0; //Reseting column
            row += 1; //Incrementing row / total guesses
            
            typed[0] = '\0'; //Resetting typed

            if(result == 1 || row == guesses) //If word is guesses or max guesses has been reached
            {
                gamestate = over;
            }
            message[0] = '\0'; //Resetting message
        }
        else
        {
            strcpy(message , "Not a valid Word.\0");
        }
    }

    for(int i = 0 ; i < row ; i++) //Printing highlight history and word history
    {
        for(int j = 0 ; j < length ; j++)
        {
            Color back = LIGHTGRAY;
            switch(highlightHistory[i][j])
            {
            case 0:
                back = GREEN;
                break;
            case 1:
                back = YELLOW;
                break;
            case 2:
                back = ORANGE;
                break;
            case 3:
                back = DARKGRAY;
                break;
            }

            char letter[2] = {guessHistory[i][j] , '\0'};

            int fontsize = 50;
            int textW = MeasureText(letter , 50);

            int centreX = posX[j] + (buttonW/2 + textW)/2;
            int centreY = posY[i] + fontsize/2;

            DrawRectangle(posX[j] , posY[i] , buttonW , buttonH , back);
            DrawText(letter , centreX , centreY , fontsize , BLACK);
        }
    }

    for(int i = 0 ; i < col ; i++) //Displaying currently typed guess
    {
        char letter[2] = {typed[i] , '\0'};

        int fontsize = 50;
        int textW = MeasureText(letter , 50);

        int centreX = posX[i] + (buttonW/2 + textW)/2;
        int centreY = posY[row] + fontsize/2; 

        DrawText(letter , centreX , centreY , fontsize , BLACK);
    }

    if(gamestate == over) 
    {
        if(result == 1) //If game was won
        {
            sprintf(message , "You guessed correctly in %d guesses." , row);
            fetchHighscore();
            if(row < highscore[currentLetter])
            {
                setHighscore(row);
                sprintf(message , "You guessed correctly in %d guesses. NEW HIGHSCORE!" , row);
            }
        }
        else
        {
            sprintf(message , "You could not guess the word %s" , currentWord);
        }

        //Displaying home button

        int textW = MeasureText("HOME" , 50);
        
        Vector2 mouse = GetMousePosition();

        int centreX = GetScreenWidth()/2 - (textW + 2*spacing)/2;
        Rectangle home = {centreX , posY[guesses-1] + 150 , textW + 2*spacing , 50};
        DrawRectangleRec(home , LIGHTGRAY);

        if(button(home))
        {
            DrawRectangleRec(home , GREEN);
            currentGuess = 2;
            currentLetter = 2;
            guesses = 6;
            length = 5;
            gamestate = menu;
        }
        else if(CheckCollisionPointRec(mouse , home))
        {
            DrawRectangleRec(home , DARKGRAY);
        }
        drawCentered("HOME" , 30 , posY[guesses-1] + 160 , COL_TEXT);
    }
    drawCentered(message , 25 , posY[guesses-1] + 100, COL_TEXT);
}

// ------------------------ MAIN FUNCTION --------------------------
int main()
{
    srand(time(NULL));

    InitWindow(screenWidth, screenHeight , "Word Guessing Game");
    SetTargetFPS(60);
    
    gamestate = menu;

    fetchHighscore();

    while(!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(COL_BG);
            if(gamestate == menu)
            {
                drawMenu();
            }
            else if(gamestate == start || gamestate == over)
            {
                gameStart();
            }
        EndDrawing();
    }
    
    CloseWindow();

    return 0;

}