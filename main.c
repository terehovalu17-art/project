#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "image.h"
#include "filters.h"


typedef enum {F_CROP, F_GS, F_NEG, F_SHARP} FType;
typedef struct FilterNode {
    FType type;
    double param1;
    int param_int;
    struct FilterNode *next;
} FilterNode;

int is_number(const char *s){
    if(!s) return 0;
    char *end; 
    strtod(s, &end);
    return *end == '\0';
}

int main(int argc, char **argv){
    if(argc < 2){
        return 0;
    }
    if(argc == 2){
        return 0;
    }
    const char *inpath = argv[1];
    const char *outpath = argv[2];
    
    FilterNode *head = NULL, *tail = NULL;
    int i = 3;
    while(i < argc){
        if(argv[i][0] != '-'){ fprintf(stderr, "Unexpected token: %s\n", argv[i]); return 1; }
        const char *opt = argv[i] + 1;
        if(strcmp(opt,"crop")==0){
            if(i+2 >= argc) { fprintf(stderr,"-crop needs 2 args\n"); return 1; }
            int w = atoi(argv[i+1]);
            int h = atoi(argv[i+2]);
            FilterNode *n = malloc(sizeof(FilterNode));
            n->type = F_CROP; n->param1 = 0; n->param_int = (w<<16) ^ (h & 0xFFFF); n->next = NULL;
            if(!head) head = tail = n; else { tail->next = n; tail = n; }
            i += 3;
        } else if(strcmp(opt,"gs")==0){
            FilterNode *n = malloc(sizeof(FilterNode)); n->type = F_GS; n->param1=0;n->param_int=0;n->next=NULL;
            if(!head) head = tail = n; else { tail->next=n; tail=n; } i++;
        } else if(strcmp(opt,"neg")==0){
            FilterNode *n = malloc(sizeof(FilterNode)); n->type = F_NEG; n->param1=0;n->param_int=0;n->next=NULL;
            if(!head) head = tail = n; else { tail->next=n; tail=n; } i++;
        } else if(strcmp(opt,"sharp")==0){
            FilterNode *n = malloc(sizeof(FilterNode)); n->type = F_SHARP; n->param1=0;n->param_int=0;n->next=NULL;
            if(!head) head = tail = n; else { tail->next=n; tail=n; } i++;
        } else {
            fprintf(stderr, "Unknown filter: %s\n", opt);
            return 1;
        }
    }

    Image *img = image_load_bmp(inpath);
    if(!img) { fprintf(stderr, "Failed to load BMP: %s\n", inpath); return 1; }

    if(!head){
        if(!image_save_bmp(outpath, img)){ fprintf(stderr,"Failed to save output\n"); image_free(img); return 1; }
        image_free(img);
        printf("Saved copy to %s\n", outpath);
        return 0;
    }

    Image *cur = img;
    FilterNode *fn = head;
    while(fn){
        Image *nextimg = NULL;
        switch(fn->type){
            case F_CROP: {
                int packed = fn->param_int;
                int w = (packed >> 16) & 0xFFFF;
                int h = packed & 0xFFFF;
                nextimg = filter_crop(cur, w, h);
                break;
            }
            case F_GS: nextimg = filter_gs(cur); break;
            case F_NEG: nextimg = filter_neg(cur); break;
            case F_SHARP: nextimg = filter_sharp(cur); break;
            default: break;
        }
        if(!nextimg){ fprintf(stderr,"Filter failed\n"); image_free(cur); return 1; }
        if(cur != img) image_free(cur);
        cur = nextimg;
        fn = fn->next;
    }
    
    if(!image_save_bmp(outpath, cur)){ fprintf(stderr,"Failed to save output\n"); image_free(cur); return 1; }
    image_free(cur);
    
    fn = head;
    while(fn){ FilterNode *tmp = fn->next; free(fn); fn = tmp; }
    printf("Saved to %s\n", outpath);
    return 0;
}
 Image *cur = img;
    FilterNode *fn = head;
    while(fn){
        Image *nextimg = NULL;
        switch(fn->type){
            case F_CROP: {
                int packed = fn->param_int;
                int w = (packed >> 16) & 0xFFFF;
                int h = packed & 0xFFFF;
                nextimg = filter_crop(cur, w, h);
                break;
            }
            case F_GS: nextimg = filter_gs(cur); break;
            case F_NEG: nextimg = filter_neg(cur); break;
            case F_SHARP: nextimg = filter_sharp(cur); break;
            case F_EDGE: nextimg = filter_edge(cur, fn->param1); break;
            case F_MED: nextimg = filter_med(cur, fn->param_int); break;
            case F_BLUR: nextimg = filter_blur(cur, fn->param1); break;
            case F_CRYS: nextimg = filter_crys(cur, fn->param_int); break;
            case F_GLASS: nextimg = filter_glass(cur, fn->param_int); break;
            default: break;
        }
        if(!nextimg){ fprintf(stderr,"Filter failed\n"); image_free(cur); return 1; }
        if(cur != img) image_free(cur);
        cur = nextimg;
        fn = fn->next;
    }
    
    if(!image_save_bmp(outpath, cur)){ fprintf(stderr,"Failed to save output\n"); image_free(cur); return 1; }
    image_free(cur);
    
    fn = head;
    while(fn){ FilterNode *tmp = fn->next; free(fn); fn = tmp; }
    printf("Saved to %s\n", outpath);
    return 0;
}
