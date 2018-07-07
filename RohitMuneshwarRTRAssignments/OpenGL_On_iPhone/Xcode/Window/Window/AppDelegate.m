//
//  AppDelegate.m
//  Window
//
//  Created by rohit muneshwar on 27/05/18.
//
//

#import "AppDelegate.h"
#import "ViewController.h"
#import "MyView.h"


@implementation AppDelegate
{
@private
    UIWindow *mainWindow;
    ViewController *mainViewController;
    MyView *myView;
}

-(BOOL) application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    //get screen bounds for fullscreen
    CGRect screenBounds = [[UIScreen mainScreen] bounds];
    
    //initialize window variable corresponding to screen bounds
    mainWindow = [[UIWindow alloc] initWithFrame:screenBounds];
    
    mainViewController = [[ViewController alloc] init];
    
    [mainWindow setRootViewController:mainViewController];
    
    //initialize view variable corresponding to screen bounds
    myView = [[MyView alloc] initWithFrame:screenBounds];
    
    [mainViewController setView:myView];
    
    [myView release];
    
    //add the ViewController's view as subview to the window
    [mainWindow addSubview:[mainViewController view]];
    
    //make window key window and visible
    [mainWindow makeKeyAndVisible];
    
    return(YES);
}

-(void) applicationWillResignActive:(UIApplication *)application
{
    
}

-(void) applicationDidEnterBackground:(UIApplication *)application
{
    
}

-(void) applicationWillEnterBackground:(UIApplication *)application
{
    
}

-(void) applicationDidBecomeActive:(UIApplication *)application
{
    
}

-(void) applicationWillTerminate:(UIApplication *)application
{
    
}

-(void) dealloc
{
    //code
    [myView release];
    
    [mainViewController release];
    
    [mainWindow release];
    
    [super dealloc];
}
@end
