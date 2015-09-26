http://selainsoft.blogspot.com/2008/07/functionality-versus-capability-case-in.html
Cross wall pls.

張貼者： seLain 位於 下午 8:07
這兩天跟學妹一起在對 Java OpenCV Match Template (以下簡稱 Java OpenCV ) 這個 Tool 作一點修改.

Java OpenCV 其實是一個 Adapter, 把 OpenCV ( 著名的 Computer Vision Library ) 內的 cvMatchTemplate Function ( 用 C++ 寫的 ) 包上 Java 的外衣. 而 cvMatchTemplate 的主要能力就是比對基底影像 ( Source Image ) 與模板影像 ( Template Image ), 找出在基底影像中哪些地方存在著與模板影像相似的影像, 以及該處的位置 ( 在圖片或影像截圖上的 ).

為簡化敘述, 先不論 Java OpenCV 其實是 Adapter 而已, 假設把 Java OpenCV 跟 cvMatchTemplate 包在一起看成一個單一的 Component, 而這個 Component 提供一個 Interface 可以輸入基底影像以及模板影像, 而會回傳一個最佳的比對結果影像物件 ( Java Image Object ), 以及位置. 由於這是此 Component 唯一能夠提供的 Service, 因此就說這是 Java OpenCV 的 Functionality.

但是我們需要的是能夠提供多個比對結果的 Functionality. 例如下面的 Java OpenCV 測試圖, 在四個角落都有幾乎同樣的圓點, 但是 Java OpenCV 只會傳回最相似的一個結果而已. 而我們希望四個結果都可以傳回給我們作選擇.


由於短時間內沒有找到其他的 Tool 如此符合我們的需求, 加上 Java OpenCV 的程式碼其實不長, 因此我們去查看了一下原始碼. 在 Java OpenCV 與 cvMatchTemplate之間是使用 JNI 作連結, 而主要的 Image Matching 都是 cvMatchTemplate 進行的沒錯. cvMatchTemplate 是以逐一影像區塊進行比對的方式, 比完整張圖, 因此用了一個很大的 array 來存所有的比對結果.

view plaincopy to clipboardprint?

  1. /**...... 前面省略**/
> 2. float**match\_result = (float**)malloc(RESULT\_WIDTH\*RESULT\_HEIGHT\*sizeof(float));
> 3. /**...... 中間省略**/
> 4. jfloatArray return\_result;
> 5. return\_result = env->NewFloatArray(RESULT\_WIDTH\*RESULT\_HEIGHT);
> 6. env->SetFloatArrayRegion(return\_result, 0, RESULT\_WIDTH\*RESULT\_HEIGHT, (jfloat**)match\_result);
> 7. free(match\_result);
> 8.
> 9. return return\_result;**

<br />/**...... 前面省略**/<br />float**match\_result = (float**)malloc(RESULT\_WIDTH\*RESULT\_HEIGHT\*sizeof(float));<br />/**...... 中間省略**/<br />jfloatArray return\_result;<br />return\_result = env->NewFloatArray(RESULT\_WIDTH\*RESULT\_HEIGHT);<br />env->SetFloatArrayRegion(return\_result, 0, RESULT\_WIDTH\*RESULT\_HEIGHT, (jfloat**)match\_result);**<br />free(match\_result);<br /><br />return return\_result;<br />

而 cvMatchTemplate 回傳給 Java OpenCV 的內容其實包含了所有的比對結果, 而不是只有最佳的比對結果. 換句話說, 是在 Java OpenCV 內被做了過濾. 更嚴格來說是在 matchTemplate method 回傳之前, 在 findMaxPoint 裡面被過濾掉了, 只留下最佳的比對結果位置.

view plaincopy to clipboardprint?

  1. public Point matchTemplate(int x, int y, int w, int h, int method )
> 2.  {
> 3.   float[.md](.md) sourceImgData = getFloatPixels(sourceImgBI, x, y, w, h);
> 4.
> 5.   //Print sourceImgData
> 6.   //displayBufferedImage(getImageGrayFromFloat(sourceImgData, w, h), "sourceImgData");
> 7.
> 8.   this.cvMatchTemplateFloat = cvMatchTemplate(sourceImgData, w, h, this.templateImgData, this.TEMPLATE\_W, this.TEMPLATE\_H, method);
> 9.   RESULT\_W = w-this.TEMPLATE\_W+1;
  1. .   RESULT\_H = h-this.TEMPLATE\_H+1;
  1. .   //System.out.println("result: <"+RESULT\_W+","+RESULT\_H+">");
  1. .   //System.out.println("result.length =" + this.cvMatchTemplateFloat.length);
  1. .
  1. .   Point resultPoint = null;
  1. .   if ((method==MatchTemplate.CV\_TM\_SQDIFF) || (method==MatchTemplate.CV\_TM\_SQDIFF\_NORMED))
  1. .   {
  1. .    resultPoint = findMinPoint(x, y);
  1. .   }
  1. .   else
> 20.   {
> 21.    resultPoint = findMaxPoint(x, y);
> 22.   }
> 23.   //getMatchImage();
> 24.   //displayImages(x, y);
> 25.
> 26.   return resultPoint;
> 27.  }
> 28.
> 29.  private Point findMaxPoint(int x, int y)
> 30.  {
> 31.   int maxIndex = 0;
> 32.   float maxValue= Float.MIN\_VALUE;
> 33.   for(int i = 0 ; i< this.cvMatchTemplateFloat.length; i++)
> 34.   {
> 35.    if (this.cvMatchTemplateFloat[i](i.md)>=maxValue)
> 36.    {
> 37.     maxValue = this.cvMatchTemplateFloat[i](i.md);
> 38.     maxIndex = i;
> 39.    }
> 40.   }
> 41.   int max\_h = maxIndex / RESULT\_W;
> 42.   int max\_w = maxIndex - max\_h\*RESULT\_W;
> 43.   //System.out.println("max point: <"+(max\_w+x)+","+(max\_h+y)+">");
> 44.   return new Point(max\_w+x, max\_h+y);
> 45.  }

<br />public Point matchTemplate(int x, int y, int w, int h, int method )<br /> {<br /> float[.md](.md) sourceImgData = getFloatPixels(sourceImgBI, x, y, w, h);<br /> <br /> //Print sourceImgData<br /> //displayBufferedImage(getImageGrayFromFloat(sourceImgData, w, h), "sourceImgData");<br /> <br /> this.cvMatchTemplateFloat = cvMatchTemplate(sourceImgData, w, h, this.templateImgData, this.TEMPLATE\_W, this.TEMPLATE\_H, method);<br /> RESULT\_W = w-this.TEMPLATE\_W+1;<br /> RESULT\_H = h-this.TEMPLATE\_H+1;<br /> //System.out.println("result: <"+RESULT\_W+","+RESULT\_H+">");<br /> //System.out.println("result.length =" + this.cvMatchTemplateFloat.length);<br /> <br /> Point resultPoint = null;<br /> if ((method==MatchTemplate.CV\_TM\_SQDIFF) || (method==MatchTemplate.CV\_TM\_SQDIFF\_NORMED))<br /> {<br /> resultPoint = findMinPoint(x, y);<br /> }<br /> else<br /> {<br /> resultPoint = findMaxPoint(x, y);<br /> }<br /> //getMatchImage();<br /> //displayImages(x, y);<br /> <br /> return resultPoint;<br /> }<br /> <br /> private Point findMaxPoint(int x, int y)<br /> {<br /> int maxIndex = 0;<br /> float maxValue= Float.MIN\_VALUE;<br /> for(int i = 0 ; i< this.cvMatchTemplateFloat.length; i++)<br /> {<br /> if (this.cvMatchTemplateFloat[i](i.md)>=maxValue)<br /> {<br /> maxValue = this.cvMatchTemplateFloat[i](i.md);<br /> maxIndex = i;<br /> }<br /> }<br /> int max\_h = maxIndex / RESULT\_W;<br /> int max\_w = maxIndex - max\_h\*RESULT\_W;<br /> //System.out.println("max point: <"+(max\_w+x)+","+(max\_h+y)+">");<br /> return new Point(max\_w+x, max\_h+y); <br /> }<br />

這樣一來事情就很簡單了, 我們另外加上了一個 matchTemplateMultiple method, 使用動態決定 Threshold 的方式來過濾出最好的幾個比對結果, 因此可以獲得多個比對結果.

但我覺得有趣的是, 很顯然地 Java OpenCV Component 本身就具有偵測到所有可能比對結果的 Capability, 只是由於他只提供了 matchTemplate 的 Interface, 因此形成了 Functionality 與 Capability 不相稱的情況. 基於 Encapsulation 的原則, 我們不可能從 Interface 就得知 Java OpenCV Component 的真實 Capability, 但是透過知道 Java OpenCV 原本的 Capability, 卻使得我可以評估花較少的 Effort 去增加 Java OpenCV 的 Functionality, 變相地增加了 Java OpenCV 的 Re-usability.

之前會說希望把 Java OpenCV 看成單一 Component 而不要管 Adapter 身份是因為此 Case 有點特殊, Adapter 本身本來就可能對於原本的 Component Functionality 有遮蔽的作用. 但我認為對於一般 Component 來說也可能會有此現象發生, 如果可以有辦法偵測或是表達 Component 本身的 Capability 與 Functionality 之間的差異, 或許會是一件很有用的資訊.