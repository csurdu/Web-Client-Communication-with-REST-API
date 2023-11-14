#include <arpa/inet.h>
#include <netdb.h> /* struct hostent, gethostbyname */
#include <ctype.h>
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <stdio.h>      /* printf, sprintf */
#include <stdbool.h>
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <sys/socket.h> /* socket, connect */
#include <string.h>     /* memcpy, memset */
#include <unistd.h>     /* read, write, close */
#include <ctype.h>
#include "helpers.h"
#include "parson.h"
#include "requests.h"

#define REGISTER "/api/v1/tema/auth/register"
#define ACCESS "/api/v1/tema/library/access"
#define LOGIN "/api/v1/tema/auth/login"
#define LOGOUT "/api/v1/tema/auth/logout"
#define BOOKS "/api/v1/tema/library/books"

#define SWAP(a, b)      \
  do                    \
  {                     \
    typeof(a) temp = a; \
    a = b;              \
    b = temp;           \
  } while (0)

char *find_first_occurrence(char *str, char target)
{
  while (*str != '\0')
  {
    if (*str == target)
    {
      return str;
    }
    str++;
  }
  return NULL;
}

void reverse(char str[], int length)
{
  int start = 0;
  int end = length - 1;
  while (start < end)
  {
    SWAP(*(str + start), *(str + end));
    start++;
    end--;
  }
}

// Implementation of itoa()
char *itoa(int num, char *str, int base)
{
  int i = 0;
  bool isNegative = false;

  /* Handle 0 explicitly, otherwise empty string is printed for 0 */
  if (num == 0)
  {
    str[i++] = '0';
    str[i] = '\0';
    return str;
  }

  // In standard itoa(), negative numbers are handled only with
  // base 10. Otherwise numbers are considered unsigned.
  if (num < 0 && base == 10)
  {
    isNegative = true;
    num = -num;
  }

  // Process individual digits
  while (num != 0)
  {
    int rem = num % base;
    str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
    num = num / base;
  }

  // If number is negative, append '-'
  if (isNegative)
    str[i++] = '-';

  str[i] = '\0'; // Append string terminator

  // Reverse the string
  reverse(str, i);

  return str;
}

int main(int argc, char *argv[])
{
  int conectat, acces_librarie;
  conectat = 0;
  acces_librarie = 0;
  // char host[16] = "34.241.4.235";
  char host[16] = "34.254.242.81";
  int port = 8080;
  int socket = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
  char *util[1];
  char *cookieuri[1];
  char cookie[150];
  char com[50];
  char cheie[BUFLEN];
  char *addbook[1];
  while (1)
  {
    fgets(com, 100, stdin);
    com[strlen(com) - 1] = '\0';
    if (strcmp(com, "exit") == 0)
    {
      break;
    }
    else
    {
      socket = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
      if (strcmp(com, "register") == 0)
      {
        socket = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
        char *username = (char *)malloc(100 * sizeof(char));
        char *password = (char *)malloc(100 * sizeof(char));
        char delim;
        printf("username=");
        if (scanf("%s%c", username, &delim) != 2 || delim != '\n')
        {
          printf("Username gresit!\n");
          continue;
        }
        printf("password=");
        if (scanf("%s%c", password, &delim) != 2 || delim != '\n')
        {
          printf("Parola gresita!\n");
          continue;
        }

        JSON_Value *value = json_value_init_object();
        JSON_Object *object = json_value_get_object(value);
        json_object_set_string(object, "username", username);
        json_object_set_string(object, "password", password);
        util[0] = json_serialize_to_string(value);
        char *message = compute_post_request(host, REGISTER, "application/json", util, 1, NULL, 0, NULL);
        send_to_server(socket, message);

        if (strstr(receive_from_server(socket), "is taken") != NULL)
        {
          printf("TAKEN USERNAME! Please try another one.\n");
        }
        else
        {
          printf("DONE! You are now registred.\n");
        }
      }

      if (strcmp(com, "login") == 0)
      {
        char *username = (char *)malloc(100 * sizeof(char));
        char *password = (char *)malloc(100 * sizeof(char));

        printf("username=");
        scanf("%s", username);
        printf("password=");
        scanf("%s", password);

        JSON_Value *value = json_value_init_object();
        JSON_Object *object = json_value_get_object(value);
        json_object_set_string(object, "username", username);
        json_object_set_string(object, "password", password);
        util[0] = json_serialize_to_string(value);
        char *message = compute_post_request(host, LOGIN, "application/json", util, 1, NULL, 0, NULL);
        send_to_server(socket, message);

        char *string = strstr(receive_from_server(socket), "Cookie: ");
        if (string == NULL)
        {
          conectat = 0;
          acces_librarie = 0;
          printf("FAILED!\n");
          continue;
        }
        strtok(string, ";");
        string += 7;
        strcpy(cookie, string);
        cookieuri[0] = string;
        if (cookie != NULL)
        {
          printf("Logged in!\n");
          conectat = 1;
        }
      }
      if (strcmp(com, "enter_library") == 0)
      {
        if (conectat == 0)
        {
          printf("Not logged! ; Log in\n");
        }
        if (conectat == 1)
        {
          char *message = compute_get_request(host, ACCESS, NULL, cookieuri, 1, cheie);
          send_to_server(socket, message);
          char *string = strstr(receive_from_server(socket), "\":\"");
          printf("\n");
          if (string == NULL)
          {
            acces_librarie = 0;
            printf("Acces Denied\n");
          }
          if (string != NULL)
          {

            char *string2 = strtok(string, "\"");
            string2 = strtok(NULL, "\"");
            strcpy(cheie, string2);
            acces_librarie = 1;
            printf("Acces Granted\n");
          }
        }
      }

      if (strcmp(com, "get_books") == 0)
      {
        if (acces_librarie == 0)
        {
          printf("Enter the library\n");
        }
        if (acces_librarie == 1)
        {
          char *message = compute_get_request(host, BOOKS, NULL, cookieuri, 1, cheie);
          send_to_server(socket, message);
          char *arhiva_carti = strstr(receive_from_server(socket), "[");
          printf("%s\n", arhiva_carti);
        }
      }
      if (strcmp(com, "get_book") == 0)
      {
        if (acces_librarie == 0)

        {
          printf("Enter the library\n");
        }
        if (acces_librarie == 1)
        {
          int id_book = 0;
          char s[BUFLEN];
          char s2[100];

          printf("id=");
          scanf("%d", &id_book);
          if (isdigit(id_book) == 1)
          {
            printf("Bad format!Enter id again\n");
            printf("id=");
            scanf("%d", &id_book);
          }
          if (id_book < 0)
          {
            while (id_book < 0)
            {
              printf("Bad format!Enter id again\n");
              printf("id=");
              scanf("%d", &id_book);
            }
          }

          strcpy(s, BOOKS);
          itoa(id_book, s2, 10);
          strcat(s, "/");
          strcat(s, s2);
          char *message = compute_get_request(host, s, NULL, cookieuri, 1, cheie);
          send_to_server(socket, message);
          if (strstr(receive_from_server(socket), "No book was found!") != NULL)
          {
            printf("Book not found.Search id again!\n");
          }
          else
          {
            char *message = compute_get_request(host, s, NULL, cookieuri, 1, cheie);
            send_to_server(socket, message);
            char *received_data = receive_from_server(socket);
            char *first_occurrence = find_first_occurrence(received_data, '{');

            if (first_occurrence != NULL)
            {
              printf("%s\n", first_occurrence);
            }
          }
        }
      }

      if (strcmp(com, "add_book") == 0)
      {
        char p_string[BUFLEN];
        int ok = 0;
        if (acces_librarie == 0)
        {
          printf("Enter the library!\n");
        }
        if (acces_librarie == 1)
        {
          char title[100], author[100], genre[100], publisher[100];
          int pages;
          printf("title=");
          scanf("%s", title);
          printf("author=");
          scanf("%s", author);
          printf("genre=");
          scanf("%s", genre);
          printf("publisher=");
          scanf("%s", publisher);
          printf("pages=");
          scanf("%d", &pages);
          if (isspace(title) == 1 || isspace(author) == 1 || isspace(genre) == 1 || isspace(publisher) == 1 || pages < 1)
          {
            printf("Missind field,add book again\n");
            ok = 1;
          }

          if (ok == 0)
          {
            sprintf(p_string, "%d", pages);
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "title", title);
            json_object_set_string(object, "author", author);
            json_object_set_string(object, "genre", genre);
            json_object_set_string(object, "page_count", p_string);
            json_object_set_string(object, "publisher", publisher);
            addbook[0] = json_serialize_to_string(value);

            char *message = compute_post_request(host, BOOKS, "application/json", addbook, 1, NULL, 0, cheie);
            send_to_server(socket, message);
            receive_from_server(socket);
            printf("Book added!\n");
          }
        }
      }

      if (strcmp(com, "delete_book") == 0)
      {
        int id_book = 0;
        char s[BUFLEN];
        char s2[100];
        if (acces_librarie == 0)
        {
          printf("Enter library!\n");
        }

        if (acces_librarie == 1)
        {
          printf("id=");
          scanf("%d", &id_book);
          if (id_book < 0)
          {
            while (id_book < 0)
            {
              printf("Bad format!Enter id again\n");
              printf("id=");
              scanf("%d", &id_book);
            }
          }
          strcpy(s, BOOKS);
          itoa(id_book, s2, 10);
          strcat(s, "/");
          strcat(s, s2);
          char *message = compute_del_request(host, s, NULL, cookieuri, 1, cheie);
          send_to_server(socket, message);

          char *del = strstr(receive_from_server(socket), "No book was delelted!");
          if (del != NULL)
          {
            printf("NO BOOK! Entered id is not valid!\n");
            continue;
          }
          printf("Book Deleted!!\n");
        }
      }
      if (strcmp(com, "logout") == 0)
      {
        if (conectat == 0)
        {
          printf("Not logged in!\n");
        }
        if (conectat == 1)
        {
          // comanda se realizeaza doar daca suntem logati deja
          char *message = compute_get_request(host, LOGOUT, NULL, cookieuri, 1, cheie);
          send_to_server(socket, message);
          receive_from_server(socket);

          conectat = 0;
          acces_librarie = 0;
          printf("Logging out !Thank you!\n");
        }
      }
      close_connection(socket);
    }
  }
}
