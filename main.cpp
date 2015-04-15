#include <bits/stdc++.h>
#include "PerlinNoise2D.h"
// #include "ValueNoise2D.h"

using namespace std;
int main()
{
  FILE *bmp;

  int size = 128;
  unsigned int bitmap[size*size];
  unsigned char bmpHeader[54] = {
    'B', 'M', /* [ 0] ファイルタイプ */
    246, 0, 0, 0, /* [ 2] ファイルサイズ */
    0, 0, 0, 0, /* [ 6] 予約 */
    54, 0, 0, 0, /* [10] ビットマップデータのシーク位置 */
    40, 0, 0, 0, /* [14] ここから始まるヘッダの高さ */
    128, 0, 0, 0, /* [18] ビットマップの幅 */
    128, 0, 0, 0, /* [22] ビットマップの高さ */
    0x01, 1, /* [26] プレーン数 */
    32, 0, /* [28] 1ピクセルあたりのビット数 （課題が4バイト指定されていたので32bitに変更） */ 
    0, 0, 0, 0, /* [30] 圧縮タイプ */
    192, 0, 0, 0, /* [34] ビットマップデータの長さ */
    0x20, 0x2e, 0, 0, /* [38] 度(px/m) */
    0x20, 0x2e, 0, 0, /* [42] 度(px/m) */
    0, 0, 0, 0, /* [46] カラーインデックス数 */
    0, 0, 0, 0, /* [50] 重要なカラーインデックス数 */
  };
  bmp = fopen("perlin.bmp", "wb");
  // ValueNoise2D perlin;
  PerlinNoise2D perlin;
  // PerlinNoise1D perlin;
  // perlin.setRange(size, size);
  perlin.setPersistence(0.65);
  perlin.setOcterve(1);

  perlin.generate();
  perlin.printData();
  for(int i = 0; i < size;i++){
    for(int j = 0; j < size; j++){
      int val = 256*((perlin.get(i*size+j)+1.0)/2.0);
      bitmap[i*size+j] = 256*256*val + 256*val + val; 
    }
  }
  fseek(bmp, 0L, SEEK_SET);
  fwrite(bmpHeader, 1, 54, bmp);
  fwrite(bitmap, 4, size*size, bmp);
  fclose(bmp);
  return 0;
}
