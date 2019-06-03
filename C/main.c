#include <stdio.h>
#include <string.h>


void newCode(char *buf, int size);


int main() {
    char touchCode[5] = "";
    char password[5] = "1323";
    char key;
    int i = 0;
    int lenCode= sizeof(password)/sizeof(password[0]);

    //infinite loop
    while(1)
    {
        printf("\nEnter any character:  ");
        //reads a single character
        scanf(" %c", &key);

        if ((key >= 48) && (key <= 57)){    // If input is between 0-9
            i = 0;
            while (touchCode[i] != '\0') {
                i++;
            }

            touchCode[i] = key;
            printf("\nCurrent code: %s",touchCode);

            if (i == lenCode-2){    // If password attempt is finished
                printf("\n----------------------\n");
                if (strcmp(touchCode,password) == 0){
                    printf("\nSUCCESS");
                    newCode(password,lenCode);
                } else{
                    printf("\nWRONG CODE");
                }
                memset(touchCode,0,lenCode);    // Resets code
                printf("\n----------------------\n");
            }
        } else{
            printf("\nEXIT PROGRAM");
            break;
        }
    }
    return 0;
}

// Enables user to change password
void newCode(char *buf, int size) {
    char key;
    int j=0;

    memset(buf,0,size); // Resets password

    while (j < size-2){
        printf("\nNEW PASSWORD:  ");
        scanf(" %c", &key);
        j = 0;
        if ((key >= 48)&& (key <= 57)){

            while (buf[j] != '\0') {
                j++;
            }
            buf[j] = key;
            printf("\nCurrent Password: %s",buf);
        }
    }
    buf[strlen(buf)] = '\0'; // Sets last character to '\0'
}
