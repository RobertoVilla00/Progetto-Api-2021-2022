#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <assert.h>
#define BUFLEN 100
#define ALPHA 64
#define OCC_EMPTY 255

typedef struct dict{
    struct dict *right;
    struct dict *left;
    struct dict *next;
    char *word;
    unsigned char flag;
}Dict;


typedef struct pipe{
    struct pipe *right;
    struct pipe *left;
    struct pipe *parent;
    char symbol;
    unsigned char count;
}Pipe;

typedef struct filter{
    struct filter *next;
    char symbol;
    unsigned char count;
    unsigned char flag;
}Filter;

typedef struct occurrences{
    char symbol;
    unsigned char count;
}Occs;

int g_length = 0;
Dict *g_word_list=NULL;

Dict *dict_new(char *word, Dict *left, Dict *right, unsigned char flag){
    Dict *new= malloc(sizeof(Dict));
    new->word= malloc(g_length+1);
    strcpy(new->word,word);
    new->left=left;
    new->right=right;
    new->flag=flag;
    new->next=NULL;

    if(flag==1) {
        new->next = g_word_list;
        g_word_list = new;
    }
    return new;
}

Pipe *pipe_new(char symbol, Pipe *left, Pipe *right, Pipe *parent){
    Pipe *temp= malloc(sizeof(Pipe) );
    temp->symbol=symbol;
    temp->left=left;
    temp->right=right;
    temp->parent=parent;
    return temp;
}

Dict ** dict_get(Dict **o, char *s, unsigned char flag){
    Dict **itr=o;
    while(*itr!=NULL){
        if(strcmp(s,(*itr)->word)<0){
            itr=&(*itr)->left;
        }
        else{
            itr=&(*itr)->right;
        }
    }
    if(!*itr){
        *itr= dict_new(s,NULL,NULL,flag);
    }
    return itr;
}

Dict *dict_insert(Dict *dict, char *word, unsigned char flag){
    dict_get(&dict,word, flag);
    return dict;
}

Pipe ** pipe_get(Pipe **o, char s, Pipe **parent){
    Pipe **itr=o;
    *parent=NULL;
    while(*itr!=NULL && (*itr)->symbol!=s){
        *parent=*itr;
        if(s<(*itr)->symbol){
            itr=&(*itr)->left;
        }
        else{
            itr=&(*itr)->right;
        }
    }
    if(!*itr){
        *itr= pipe_new(s, NULL, NULL, *parent);
        (*itr)->count=0;
    }
    return itr;
}

Pipe *positionPipe_insert(Pipe *position_pipe, char symbol, int pos){
    Pipe *parent;
    Pipe **node= pipe_get(&position_pipe, symbol, &parent);
    while((*node)->count!=pos+1 && (*node)->count!=0 ){
        node= pipe_get(&(*node)->right, symbol, &parent);
    }
    (*node)->count=pos+1;
    return position_pipe;
}

void deallocate_dict(Dict *dict){
    if(!dict){
        return;
    }
    deallocate_dict(dict->right);
    deallocate_dict(dict->left);
    free(dict->word);
    free(dict);
}


void deallocate_occurrences(Pipe *o){
    if(!o){
        return;
    }
    deallocate_occurrences(o->right);
    deallocate_occurrences(o->left);
    free(o);
}

void deallocate_filter(Filter *p){
    for (Filter * itr = p; itr != NULL;) {
        Filter * next = itr->next;
        free(itr);
        itr = next;
    }
}

int contains(Dict *dict, char *word){ // controlla se la parola è presente nel dizionario
    while(dict!=NULL && strcmp(dict->word,word)!=0){
        if(strcmp(word,dict->word)<0){
            dict=dict->left;
        }
        else{
            dict=dict->right;
        }
    }
    if(!dict) return -1;
    return 1;
    /*
    if(!dict){
        return -1;
    }
    if(strcmp(word,dict->word)==0){
        return 1;
    }
    if(strcmp(word,dict->word)<0){
        return contains(dict->left,word);
    }
    else{
        return contains(dict->right, word);
    }
     */
}


Pipe *pipe_insert(Pipe *o, char s){
    Pipe *parent;
    Pipe **node= pipe_get(&o, s, &parent);
    (*node)->count++;
    return o;
/*
    Pipe *p=o;
    while(p!=NULL){
        if(p->symbol==s){
            p->count++;
            return o;
        }
        p=p->next;
    }
    if(!o) {
        o = (Pipe *) malloc(sizeof(Pipe));
        o->symbol=s;
        o->count=1;
        o->next=NULL;
        return o;
    }
    Pipe *itr=o;
    while(itr!=NULL){
        if(itr->next==NULL) {
            Pipe *cn = (Pipe *) malloc(sizeof(Pipe));
            cn->symbol = s;
            cn->count = 1;
            cn->next = itr->next;
            itr->next = cn;
            return o;
        }
        itr=itr->next;
    }
    return o;
    */
}

Occs *occ_get(Occs *occs, char symbol){
    int j=0;
    for(; occs[j].symbol!='?' && occs[j].symbol!=symbol;j++);
    return &occs[j];
}

void occ_init(Occs *occs){
    for(int i=0;i<g_length+1;i++){       // TODO mettere g_length  ALPHA
        occs[i].symbol='?';
        occs[i].count=0;
    }
}

void fill_occurrences(const char r[], Occs *occurrences) {
    occ_init(occurrences);
    for (int i = 0; i < g_length; i++) {
        Occs *temp=occ_get(occurrences,r[i]);
        temp->count++;
        temp->symbol=r[i];
    }
}

int search(char c, Pipe *o){
    while(o!=NULL && o->symbol!=c){
        if(c<o->symbol){
            o=o->left;
        }
        else{
            o=o->right;
        }
    }
    if(o!=NULL) return o->count;
    return 0;
}

int search_pipe(char c, Pipe *o){
    while(o!=NULL && o->symbol!=c){
        if(c<o->symbol){
            o=o->left;
        }
        else{
            o=o->right;
        }
    }
    if(o!=NULL) return 1;
    return 0;
}

int count_plus(char p[], char r[],int n, char c, int length){
    int count=0;
    for(int i=n+1;i<length;i++){
        if(p[i]==c && r[i]==c)
            count++;
    }
    return count;
}

int count(char c, int n, char p[], char r[], int length){
    int cont=0;
    for(int i=0;i<n;i++){
        if(p[i]==c){
            cont++;
        }
    }
    return cont+count_plus(p,r,n,c,length);
}

Filter *filter_new(char symbol,int count, int flag, Filter *next){
    Filter *temp;
    temp= (Filter *) malloc(sizeof (Filter));
    temp->count=0;  // =count
    temp->symbol=symbol;
    temp->flag=0;  // =flag
    temp->next=NULL;
    return temp;
}

Filter *filter_search(Filter **f, char symbol){
    Filter *itr=*f;
    if(!itr){
        *f=filter_new(symbol,0,0,NULL);
        return *f;
    }
    while(itr->next!=NULL){
        if(itr->symbol==symbol){
            return itr;
        }
        itr=itr->next;
    }
    if(itr->symbol==symbol){
        return itr;
    }
    itr->next=filter_new(symbol,0,0,NULL);
    return itr->next;
}

Filter *filter_union(Filter *filter, Filter *newFilter){
    Filter *itr=newFilter;
    while(itr!=NULL){
        Filter *old=filter_search(&filter, itr->symbol);
        if((old->flag==0 && itr->count >= old->count)|| itr->flag==1 ){
            old->flag=itr->flag;
            old->count=itr->count;
        }
        itr=itr->next;
    }
    return filter;
}

Filter *filter_update(Filter *o, char s, char output) {
    Filter *old=NULL;
    old=filter_search(&o,s);
    if(output=='/'){
        old->flag=1;
    }
    else{
        old->count++;
    }
    return o;
}

Dict *dict_minimum(Dict *dict){
    while(dict->left!=NULL){
        dict=dict->left;
    }
    return dict;
}

Pipe *occurrences_minimum(Pipe *o){
    if(!o){
        return NULL;
    }
    while(o->left!=NULL){
        o=o->left;
    }
    return o;
}

/*
Dict *dict_successor(Dict *dict){
    if (dict->right) {
        return dict_minimum(dict->right);
    }

    Dict* pre = dict->parent;
    while (pre && dict == pre->right) {
        dict = pre;
        pre = pre->parent;
    }

    return pre;
}
*/

void show_dict_inorder(Dict *p){
    if(p!=NULL){
        show_dict_inorder(p->left);
        if(p->flag==1){
            printf("%s\n", p->word);  // visualizza l'informazione
        }
        show_dict_inorder(p->right);
    }
}

Pipe *occurrences_successor(Pipe *o){
    if (o->right) {
        return occurrences_minimum(o->right);
    }

    Pipe * pre = o->parent;
    while (pre && o == pre->right) {
        o = pre;
        pre = pre->parent;
    }

    return pre;
}

int char_to_num(char symbol){
    if(symbol>64 && symbol<91){
        return symbol-65;
    }
    if(symbol>96 && symbol<123){
        return symbol -71;
    }
    if(symbol>47 && symbol<58){
        return symbol + 4;
    }
    if(symbol==95){
        return 62;
    }
    if(symbol==45){
        return 63;
    }
    return 0;
}
// O(64kn)
int filter_word(int matrix[g_length][ALPHA], char word[], char positionFilter[]) {
    // O(k)
    for (int i = 0; i < g_length; i++) {
        if (positionFilter[i] != '?' && positionFilter[i] != word[i]) {
            return 0;
        }
    }
    for (int i = 0; i < g_length; i++) {
        if (matrix[i][char_to_num(word[i])] == 1) {
            return 0;
        }
    }
    int count;
    char c;
    for (int i = 0; i < g_length; i++) {
        count=0;
        c = word[i];
        for (int j = 0; j < g_length; j++) {
            if (c == word[j]) {
                count++;
            }
        }
        // .....
    }
    return 1;
}
    /*
    int count[g_length*2];
    int n_cont=2;  // lunghezza vettore count
    int cont=0;
    int symbol;

    for (int i = 0; i < g_length; i++) {
        symbol= char_to_num(word[i]);
        if(i==0){
            count[0]= char_to_num(word[0]);
            count[1]=1;
        }
        else {
            while (cont < n_cont && count[cont] != symbol) {
                cont = cont + 2;
            }
            if (cont >= n_cont) {
                count[n_cont] = symbol;
                count[n_cont + 1] = 1;
            } else {
                count[cont + 1]++;
            }
        }
        if(matrix[i][char_to_num(word[i])]==1){
            return 0;
        }
    }
    cont=0;
    for (int i = 0; i < g_length; i++){
        while (cont < n_cont && count[cont] != symbol) {
            cont = cont + 2;
        }
        if (cont < n_cont) {
            if(count[cont+1]<matrix[g_length][count[cont]]){
                return 0;
            }
        }
    }
    return 1;
     */


/*Dict* dict_remove_node(Dict *root, Dict* to_rem) {
    Dict* to_free = NULL;
    Dict* subtree = NULL;
    Dict* newRoot = root;

    if (!to_rem->left || !to_rem->right) {
        to_free = to_rem;
    }
    else {
        to_free = dict_successor(to_rem);
    }

    if (to_free->left) {
        subtree = to_free->left;
    }
    else {
        subtree = to_free->right;
    }

    if (subtree) {
        subtree->parent = to_free->parent;
    }

    if (!to_free->parent) {
        newRoot = subtree;
    }
    else if (to_free == to_free->parent->left) {
        to_free->parent->left = subtree;
    }
    else {
        to_free->parent->right = subtree;
    }

    if (to_free != to_rem) {
        to_rem->word = to_free->word;
    }

    return newRoot;
}
*/

void filter_dict(Dict **wordList, int *dict_dimension, char positionFilter[], int matrix[g_length][ALPHA]){
    for (Dict **itr=wordList; *itr != NULL;) {
        //printf("%s, ",(*itr)->word );
        if (!filter_word( matrix, (*itr)->word,positionFilter)) {
            //printf("D, ");
            (*dict_dimension)--;
            (*itr)->flag = 0;
            *itr=(*itr)->next;
            //remainWords = dict_remove_node(remainWords,itr);
        }
        else{
            itr=&(*itr)->next;
        }

    }
    //printf("\n");
}

/*Dict* dict_copy(Dict * source){
    Dict *dest = NULL;
    for (Dict *itr=dict_minimum(source); itr != NULL; itr = dict_successor(itr)) {
        dest = dict_insert(dest, itr->word);
    }

    return dest;
}
*/

int read_str(char *str) {
    int i = 0;
    for(char c; (c = getchar_unlocked()) != '\n'; i++) {
        if (c == EOF) return 1;

        str[i] = c;
    }
    str[i] = '\0';

    return 0;
}

void insertion_loop_filtered(Dict *possibleWords, int *dict_dimension, int *dict_dimension_permanent, int matrix[g_length][ALPHA], char positionFilter[]){
    char command[BUFLEN];
    read_str(command);
    do {
        //char *word = (char *) malloc(strlen(command) + 1);
        //strcpy(word, command);
        (*dict_dimension)++;
        (*dict_dimension_permanent)++;
        if (!filter_word(matrix ,command,positionFilter)) {
            possibleWords = dict_insert(possibleWords, command,0);
            (*dict_dimension)--;
        }
        else {
            possibleWords = dict_insert(possibleWords, command,1);
            //remainWords= dict_insert(remainWords, word);
        }
        read_str(command);
    } while (strcmp(command, "+inserisci_fine") != 0);
}

void insertion_loop(Dict * possibleWords, int *dict_dimension, int *dict_dimension_permanent){
    char command[BUFLEN];
    read_str(command);
    do {
        //char *word = (char *) malloc(strlen(command) + 1);
        //strcpy(word, command);
        (*dict_dimension)++;
        (*dict_dimension_permanent)++;
        possibleWords = dict_insert(possibleWords, command,1);
        //remainWords= dict_insert(remainWords, word);
        read_str(command);
    } while (strcmp(command, "+inserisci_fine") != 0);
}

Filter *reset_filter(Filter *f){
    Filter *itr=f;
    while(itr!=NULL){
        itr->symbol='?';
        itr->count=0;
        itr->flag=0;
        itr=itr->next;
    }
    return f;
}

void reset_dict_h(Dict *dict, Dict **word_list){
    if (!dict) return;

    dict->flag = 1;
    Dict *temp = *word_list;
    *word_list = dict;
    (*word_list)->next = temp;

    reset_dict_h(dict->left, word_list);
    reset_dict_h(dict->right, word_list);
}


void reset_dict(Dict *dict, Dict **word_list) {
    *word_list = NULL;
    reset_dict_h(dict, word_list);
}

void reset_matrix(int matrix[g_length+1][ALPHA]){
    for(int i=0;i<g_length+1;i++){
        for(int j=0;j<ALPHA;j++){
            matrix[i][j]=0;
        }
    }
}


void update_matrix_zero(int m[g_length+1][ALPHA], int row, int column){
    for(int i=0;i<ALPHA;i++){
        m[row][i]=1;
    }
}

void update_matrix_one(int m[g_length+1][ALPHA], int column){
    for(int i=0;i<g_length;i++){
        m[i][column]=1;
    }
}



int main() {
    /*
    // Test inserimento
    List *l = NULL;
    l = list_insert(l, "d");
    l = list_insert(l, "c");
    l = dict_insert(l, "a");
    show_dict_inorder(l);
     */

    Dict *possibleWords = NULL;
    //Dict *remainWords=NULL;
    Pipe *positionPipe=NULL;
    Filter *filter=NULL;
    char command[BUFLEN];
    int attempts;
    int eof=0;
    int dict_dimension=0;
    int dict_dimension_permanent=0;
    read_str(command);
    g_length = (int) strtol(command, NULL, 10);  // ricevo il parametro k
    Occs occurrences[g_length+1];
    char positionFilter[g_length];
    int cont_min[ALPHA];

    int positionExact[ALPHA];
    int matrix[g_length+1][ALPHA];
    for(int i=0;i<ALPHA;i++){
        positionExact[i]=0;    // ? indica che non si conosce la lettera in posizione i
    }


    reset_matrix(matrix);



    char reference[BUFLEN];
    eof=read_str(command);

    do {
        dict_dimension++;
        dict_dimension_permanent++;
        possibleWords = dict_insert(possibleWords, command,1); // ricevo le parole del dizionario
        //show_dict_inorder(possibleWords);
        //printf("\n");

        // TODO: Migliorare
        eof=read_str(command);


    } while (strcmp(command, "+nuova_partita") != 0);
    //remainWords=list_copy(possibleWords);
    //show_dict_inorder(remainWords);
    //printf("\n");
    eof=0;
    do {
        //remainWords = dict_copy(possibleWords);
        for(int i=0;i<g_length;i++){
            positionFilter[i]='?';    // ? indica che non si conosce la lettera in posizione i
        }
        dict_dimension=dict_dimension_permanent;
        reset_dict(possibleWords, &g_word_list);
        reset_matrix(matrix);
        for(int i=0;i<ALPHA;i++){
            positionExact[i]=0;    // ? indica che non si conosce la lettera in posizione i
        }
        for(int i=0;i<ALPHA;i++){
            cont_min[i]=0;    // ? indica che non si conosce la lettera in posizione i
        }



        positionPipe=NULL;
        filter=NULL;
        eof = read_str(reference);
        fill_occurrences(reference,occurrences);  // riempio le occorrenze della parola
        //show_occurrences(occurrences);

        eof = read_str(command);    // ricevo il numero di tentativi
        attempts = (int) strtol(command, NULL, 10);

        while (attempts > 0) {
            eof = read_str(command);   // ricevo la parola da confrontare
            if(eof){
                return 0;
            }
            if (strcmp(command, "+inserisci_inizio") == 0) {
                insertion_loop_filtered(possibleWords,&dict_dimension,&dict_dimension_permanent,matrix, positionFilter);
                //insertion_loop(possibleWords, &dict_dimension, &dict_dimension_permanent);
                //filter_dict(possibleWords,&dict_dimension,filter,positionPipe,positionFilter);
                //show_dict_inorder(possibleWords);

            } else if (strcmp(command, "+stampa_filtrate") == 0) {
                show_dict_inorder(possibleWords);
            } else {
                if (contains(possibleWords,command) == -1) {   // controllo se la parola è contenuta nel dizionario
                    printf("not_exists\n");
                } else if (strncmp(command, reference, g_length) ==0) {//controlla se la parola inserita coincide con quella di riferimento
                    printf("ok");
                    break;
                } else {
                    for (int i = 0; i < g_length;) {
                        int temp=char_to_num(command[i]);// eseguo il check un carattere alla volta
                        if (command[i] == reference[i] && command[i] != '\0') {
                            printf("+");
                            positionFilter[i]=command[i];
                            cont_min[temp]++;
                            update_matrix_zero(matrix,i, char_to_num(command[i]));
                        } else if (occ_get(occurrences,command[i])->count >
                                   count(command[i], i, command, reference, g_length)) {
                            printf("|");
                            cont_min[temp]++;
                            matrix[i][temp]=1;

                        } else {
                            printf("/");
                            if(cont_min[temp]==0){
                                update_matrix_one(matrix,temp);
                            }
                            else{
                                matrix[i][temp]=1;
                                //int var=0;
                                //for(int cont=0;cont<i;cont++){
                                //  if(matrix[cont][temp]!=1){
                                //     var++;
                                // }
                                //}
                                positionExact[temp]=cont_min[temp];
                                int var=0;
                                for(int itr=0;itr<g_length;itr++){
                                    if(matrix[itr][temp]!=1){
                                        var++;
                                    }
                                }
                                if(var==positionExact[temp]){
                                    for(int itr=0;itr<g_length;itr++){
                                        if(matrix[itr][temp]==0){
                                            update_matrix_zero(matrix,itr, char_to_num(command[i]));
                                            positionFilter[itr]=command[i];
                                        }
                                    }
                                }
                            }
                        }
                        i = i + 1;

                    }
                    // TODO: controllare se numero minimo è maggiore di quello già salvato
                    for (int i = 0; i < ALPHA; i++) {
                        if(matrix[g_length][i]==positionExact[i]){
                            for(int j=0;j<g_length;j++){
                                if(matrix[j][i]==0){
                                    matrix[j][i]=2;
                                }
                            }
                        }
                    }
                    // unione filtri
                    if(dict_dimension>1) {
                        filter_dict(&g_word_list, &dict_dimension,positionFilter, matrix);
                    }
                    printf("\n%d\n", dict_dimension);
                    // printf("\n");
                    attempts--;
                }
            }
        }

        if (attempts == 0) {
            printf("ko");
        }

        while (1) {
            eof = read_str(command);
            if (strcmp(command, "+nuova_partita") == 0) {
                printf("\n");
                //filter = reset_filter(filter);
                //positionPipe = reset_positionPipe(positionPipe);
                //deallocate_dict(remainWords);
                occ_init(occurrences);
                deallocate_filter(filter);
                deallocate_occurrences(positionPipe);
                break;
            } else if (strcmp(command, "+inserisci_inizio") == 0) {
                insertion_loop(possibleWords, &dict_dimension, &dict_dimension_permanent);
            }else if(eof){
                break;
            }
            else {
                assert(0 && "invalid command");
            }
        }
    }
    while (!eof);
    //deallocate_dict(possibleWords);
    //deallocate_dict_no_str(remainWords);
    //deallocate_occurrences(positionPipe);
    //deallocate_filter(filter);
    return 0;
}
