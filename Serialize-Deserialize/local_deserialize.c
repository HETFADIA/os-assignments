char arr[501][501];
    int deserializelen = 0;
    int counter = 0;
    memset(arr, '\0', sizeof(arr));
    int serializelen = strlen(ch);
    char separator = '#';
    for (int i = 0; i < serializelen; i++)
    {
        if (ch[i] == separator)
        {
            deserializelen++;
            counter = 0;
            continue;
        }
        arr[deserializelen][counter++] = ch[i];
    }
    deserializelen++;
