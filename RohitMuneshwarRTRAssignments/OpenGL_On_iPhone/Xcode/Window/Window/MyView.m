//
//  MyView.m
//  Window
//
//  Created by rohit muneshwar on 10/06/18.
//
//

#import "MyView.h"

@implementation MyView
{
    NSString *centralText;
}


-(id) initWithFrame:(CGRect)frameRect
{
    self = [super initWithFrame:frameRect];
    
    if(self)
    {
        //initialization code here
        
        //set scene's background color
        [self setBackgroundColor:[UIColor whiteColor]];
        
        centralText=@"Hello iOS !!!";
        
        //GESTURE RECOGNITION
        //tap gesture code
        UITapGestureRecognizer *singleTapGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onSingleTap:)];
        
        [singleTapGestureRecognizer setNumberOfTapsRequired:1];
        
        [singleTapGestureRecognizer setNumberOfTouchesRequired:1]; //touch of 1 finger
        
        [singleTapGestureRecognizer setDelegate:self];
        
        [self addGestureRecognizer:singleTapGestureRecognizer];
        
        //double tap gesture recognizer
        UITapGestureRecognizer *doubleTapGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onDoubleTap:)];
        
        [doubleTapGestureRecognizer setNumberOfTapsRequired:2];
        
        [doubleTapGestureRecognizer setNumberOfTouchesRequired:1];
        
        [doubleTapGestureRecognizer setDelegate:self];
        
        [self addGestureRecognizer:doubleTapGestureRecognizer];
        
        
        //this will allow us to differentiate between single tap and double tap
        [singleTapGestureRecognizer requireGestureRecognizerToFail:doubleTapGestureRecognizer];
        
        //swipe gesture
        UISwipeGestureRecognizer *swipeGestureRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(onSwipe:)];
        
        [self addGestureRecognizer:swipeGestureRecognizer];
        
        //long-press gesture
        UILongPressGestureRecognizer *longPressGestureRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(onLongPress:)];
        
        [self addGestureRecognizer:longPressGestureRecognizer];
        
        
    }
    return(self);
}


 // Only override drawRect: if you perform custom drawing.
 // An empty implementation adversely affects performance during animation.
 - (void)drawRect:(CGRect)rect {
 // Drawing code
     
     //black background
     UIColor *fillColor = [UIColor blackColor];
     [fillColor set];
     UIRectFill(rect);
     
     //dictionary with kvc
     NSDictionary *dictionaryForTextAttributes = [NSDictionary dictionaryWithObjectsAndKeys:[UIFont fontWithName:@"Helvetica" size:24], NSFontAttributeName, [UIColor greenColor], NSForegroundColorAttributeName, nil];
     
     CGSize textSize=[centralText sizeWithAttributes:dictionaryForTextAttributes];
     
     CGPoint point;
     point.x = (rect.size.width/2) - (textSize.width/2);
     point.y = (rect.size.height/2) - (textSize.height/2) + 12; //12 for doc size
     
     [centralText drawAtPoint:point withAttributes:dictionaryForTextAttributes];
 }

//to become first responder
-(BOOL) acceptsFirstResponder
{
    //code
    return(YES);
}

//like keydown
-(void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    //code
    /*
    centralText = @"'touchesBegan' Event Occured";
    [self setNeedsDisplay]; //repainting
    */
}

-(void)onSingleTap: (UITapGestureRecognizer *)gr
{
    //code
    centralText = @"'onSingleTap' Event Occured";
    [self setNeedsDisplay]; //repainting
}

-(void)onDoubleTap: (UITapGestureRecognizer *)gr
{
    //code
    centralText = @"'onDoubleTap' Event Occured";
    [self setNeedsDisplay]; //repainting
}

-(void) onSwipe: (UISwipeGestureRecognizer *) gr
{
    //code
    [self release];
    exit(0);
}

-(void) onLongPress: (UILongPressGestureRecognizer *) gr
{
    //code
    centralText = @"'onLongPress' Event Occured";
    [self setNeedsDisplay]; //repainting
}

- (void) dealloc
{
    [super dealloc];
}
@end
