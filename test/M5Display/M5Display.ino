#include <M5Stack.h>    // Core2のヘッダーを準備
#include <M5GFX.h>      // M5GFXライブラリのインクルード
M5GFX lcd;              // 直接表示のインスタンスを作成（M5GFXクラスを使ってlcdコマンドでいろいろできるようにする）
M5Canvas canvas(&lcd);  // メモリ描画領域表示（スプライト）のインスタンスを作成(必要に応じて複数作成)

// 初期設定
void setup(void) {
  M5.begin();         // 本体初期化
  lcd.begin();        // 画面初期化
  // メモリ描画領域の初期設定（スプライト）
  canvas.setColorDepth(8); // カラーモード設定（書かなければ初期値16bit。24bit（パネル性能によっては18bit）は対応していれば選択可）
                           // CORE2,Basic はメモリ描画領域サイズを大きく（320x173以上?）すると16bit以上で表示されないため8bit推奨
  canvas.createSprite(lcd.width(), lcd.height()/2); // メモリ描画領域サイズを縦半分の画面サイズで準備(必要に応じて複数作成)
}

// メイン処理
void loop(void) {
  // 指定した内容をその都度直接画面に表示（チラツキあり）--------------------------------------------------
  lcd.fillRect(0, 0, lcd.width(), 120, BLACK);  // 塗り潰し四角(指定色で画面上半分を塗りつぶす)
  lcd.setTextColor(WHITE);                      // テキスト色(文字色)
  lcd.setFont(&fonts::lgfxJapanGothic_40);      // フォント
  // テキスト表示
  lcd.setCursor(0, 0);                          // テキスト表示座標
  lcd.println("直接表示");
  lcd.println("チラツキあり");
  // グラデーション表示（直接表示）
  for (int i = 0; i < 20; i++) {
    lcd.drawGradientHLine( 0, 90 + i, 107, RED, GREEN);    // 赤から緑へのグラデーション水平線
    lcd.drawGradientHLine( 107, 90 + i, 107, GREEN, BLUE); // 緑から青へのグラデーション水平線
    lcd.drawGradientHLine( 214, 90 + i, 107, BLUE, RED);   // 青から赤へのグラデーション水平線
  }

  // メモリ内に描画した画面を一括出力（チラツキなし）-------------------------------------------------------
  canvas.fillRect(0, 0, lcd.width(), 120, BLACK); // 塗り潰し四角(指定色で範囲を塗りつぶす)
  canvas.setTextColor(WHITE);                     // テキスト色(文字色)
  canvas.setFont(&fonts::lgfxJapanGothic_40);     // フォント
  // テキスト表示
  canvas.setCursor(0, 0);                         // テキスト表示座標(メモリ描画領域)
  canvas.println("ﾒﾓﾘ描画領域表示");
  canvas.println("チラツキなし");
  // グラデーション表示（メモリ描画領域）
  for (int i = 0; i < 20; i++) {
    canvas.drawGradientHLine( 0, 90 + i, 107, RED, GREEN);   // 赤から緑へのグラデーション水平線
    canvas.drawGradientHLine( 107, 90 + i, 107, GREEN, BLUE);// 緑から青へのグラデーション水平線
    canvas.drawGradientHLine( 214, 90 + i, 107, BLUE, RED);  // 青から赤へのグラデーション水平線
  }
  // メモリ描画領域を座標を指定して一括表示（スプライト）
  canvas.pushSprite(&lcd, 0, lcd.height()/2); // スプライト画面が下半分に表示されるようy座標は高さの半分を指定 （全画面なら0）
  
  delay(100); // 遅延時間
}