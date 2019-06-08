#include <stdio.h>
#include <string.h>


void newCode(int buf[], int size);


int main() {
    int touchCode[4] = {-1,-1,-1,-1};
    int password[4] = {1,3,2,3};
    int key;
    int i = 0;
    int lenCode= sizeof(password)/sizeof(password[0]);

    //infinite loop
    while(1)
    {
        printf("\nEnter any character:  ");
        //reads a single character
        scanf(" %d", &key);

        if ((key >= 0) && (key <= 9)){    // If input is between 0-9

            if(i == 4){
                i--;
            }
            touchCode[i] = key;
            i++;

            printf("\nCurrent code:");
            for (int h = 0; h < 4; ++h) {
                if(touchCode[h] != -1){
                    printf(" %d",touchCode[h]);
                }
            }
            printf("\n");
        } else if(key == 10){ // Enter
            if (i == lenCode){    // If password attempt is finished
                printf("\n----------------------\n");
                if (memcmp(touchCode, password, sizeof(password)) == 0){
                    printf("\nSUCCESS");
                    newCode(password,lenCode);
                } else{
                    printf("\nWRONG CODE");
                }
            } else{
                printf("\nWRONG CODE");
            }
            i = 0;
            memset(touchCode,-1, lenCode*4);
            printf("\n----------------------\n");
        } else if(key == 11){ // Delete
            i--;
            if(i < 0){
                i = 0;
            }
            touchCode[i] = -1;

            printf("\nCurrent code:");
            for (int h = 0; h < 4; ++h) {
                if(touchCode[h] != -1){
                    printf(" %d",touchCode[h]);
                }
            }
        } else{
            printf("\nEXIT PROGRAM");
            break;
        }
    }
    return 0;
}

// Enables user to change password
void newCode(int buf[], int size) {
    int key;
    int j=0;
    memset(buf,-1, size*4);

    while (j < size){
        printf("\nNEW PASSWORD:  ");
        scanf(" %d", &key);
        if ((key >= 0) && (key <= 9)){    // If input is between 0-9
            buf[j] = key;
            j++;

            printf("\nCurrent code:");
            for (int h = 0; h < 4; ++h) {
                if(buf[h] != -1){
                    printf(" %d",buf[h]);
                }
            }
            printf("\n");
        }
    }
}
