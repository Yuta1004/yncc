#include <stdio.h>
#include <stdlib.h>
#include "yncc.h"
#include "vector.h"

int main(int argc, char** argv){
    if(argc < 2){
        fprintf(stderr, "[ERROR] 引数が少なすぎます！\n");
        return 0;
    }

    // トークナイズ
    str_vec = vec_new(30);
    user_input = argv[1];
    token = tokenize(argv[1]);

    // 構文木生成
    program();

    // ヘッダー
    printf(".intel_syntax   noprefix\n");
    printf(".global         main\n");
    printf("\n");

    // 文字列<ヘッダー>
    for(int idx = 0; idx < 30; ++ idx) {
        char *str = (char*)vec_get(str_vec, idx);
        if(str == NULL) break;
        printf(".Lstr%d:\n", label);
        printf("        .string \"%s\"\n\n", str);
        ++ label;
    }

    // アセンブリ出力
    label = 0;
    for(int idx = 0; code[idx] != (Node*)-1; ++ idx){
        gen_asm(code[idx]);
    }
    vec_free(str_vec);
    return 0;
}
