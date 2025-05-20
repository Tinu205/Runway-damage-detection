#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

void onMouse(int event, int x, int y, int flags, void* userdata) {
    if (event == EVENT_LBUTTONDOWN) {

        vector<vector<int>>* coords = reinterpret_cast<vector<vector<int>>*>(userdata);
        
        if (coords->size() < 4) {
            coords->push_back({x, y});
            //cout << "Clicked: (" << x << ", " << y << ")" << endl;
        }
    }
}


vector<vector<int>> get_coordinates(Mat& image) {
    namedWindow("Image", WINDOW_NORMAL);

    vector<vector<int>> coordinates;
    setMouseCallback("Image", onMouse, &coordinates);

    imshow("Image", image);

    while (coordinates.size() != 4) {
        int key = waitKey(1); 
        if (key == 27) break; 
    }

    destroyWindow("Image");
    return coordinates;
}

int main() {
  string filename;
  Mat grey, blur;
  vector<Point2f> srcPts;
  Mat warped;
  Mat binaryImage;
  Mat edges;

  cout<<"Enter the image name: ";
  cin>>filename;
  
  Mat image = imread(filename);

  if (image.empty()) {
    cout << "Error opening the file" << endl;
     return -1;
  }

   
   cvtColor(image, grey, COLOR_BGR2GRAY);
   equalizeHist(grey, grey);
   medianBlur(grey, blur, 3);

   vector<vector<int>> coords = get_coordinates(image);

 
 
  for (const auto& pt : coords) {
    srcPts.push_back(Point2f(pt[0], pt[1]));
  }

  float widthTop = norm(srcPts[1] - srcPts[0]);
  float widthBottom = norm(srcPts[2] - srcPts[3]);
  float maxWidth = max(widthTop, widthBottom);

  float heightLeft = norm(srcPts[3] - srcPts[0]);
  float heightRight = norm(srcPts[2] - srcPts[1]);
  float maxHeight = max(heightLeft, heightRight);

  vector<Point2f> dstPts = {
    Point2f(0, 0),
    Point2f(maxWidth - 1, 0),
    Point2f(maxWidth - 1, maxHeight - 1),
    Point2f(0, maxHeight - 1)
  };

  Mat M = getPerspectiveTransform(srcPts, dstPts);
 
  warpPerspective(blur, warped, M, Size((int)maxWidth, (int)maxHeight));
 
  Scalar meanBrightness = mean(warped);



  if (meanBrightness[0] < 128) {
    
    cv::threshold(warped, binaryImage, 60, 255, cv::THRESH_BINARY);
  } else {
    
    cv::threshold(warped, binaryImage, 50, 255, cv::THRESH_BINARY_INV);
  }
  //cv::imshow("binaryImage",binaryImage) ;

  Canny(binaryImage, edges, 50, 150); 
  vector<vector<Point>>contours;
  vector<Vec4i>hierarchy;
  findContours(edges,contours,hierarchy,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
  Mat M_inv = M.inv(); 

for (const auto& contour : contours) {
    Rect rect = boundingRect(contour);

   
    vector<Point2f> warped_rect_pts = {
        Point2f(rect.x, rect.y),
        Point2f(rect.x + rect.width, rect.y),
        Point2f(rect.x + rect.width, rect.y + rect.height),
        Point2f(rect.x, rect.y + rect.height)
    };

    vector<Point2f> original_rect_pts;
    perspectiveTransform(warped_rect_pts, original_rect_pts, M_inv);

    
    vector<Point> polygon_pts;
    for (const auto& pt : original_rect_pts)
        polygon_pts.push_back(Point(cvRound(pt.x), cvRound(pt.y)));

    polylines(image, polygon_pts, true, Scalar(0, 0, 255), 2); 
}

imshow("Damages on the runway", image);
   while(true){
    int val = cv::waitKey(0) & 0xff;
    if(val == 'q'){
      destroyAllWindows();
      break;
    }
  }
  return 0;
}
