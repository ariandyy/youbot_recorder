/**
 * Telemetry Recorder
 * 
 * Recording:
 * - robot speed (lin_x, lin_y, ang_z)
 * + user distance
 * + user lateral position (x,y)
 * 
 * We do this in 2 parallel threads:
 * - manual text file writing (here)
 * - rosbag in launch file
 * 
 */

#include "ros/ros.h"
#include "std_msgs/Bool.h"
#include "std_msgs/Int32.h"
#include "std_msgs/Float32.h"
#include "geometry_msgs/Twist.h"

#include <fstream>
#include <ctime>
//#include "rosbag/bag.h"

// global variables
bool g_userDetected = false;
int g_userPosX = 0;
int g_userPosY = 0;
float g_userDistance = 0;
geometry_msgs::Twist g_velocity;


void callbackObjDetected(const std_msgs::Bool::ConstPtr& msg){ g_userDetected = msg->data; }
void callbackPosX(const std_msgs::Int32::ConstPtr& msg){g_userPosX = msg->data;}
void callbackPosY(const std_msgs::Int32::ConstPtr& msg){g_userPosY = msg->data;}
void callbackDistance(const std_msgs::Float32::ConstPtr& msg){g_userDistance = msg->data;}
void callbackVelocity(const geometry_msgs::Twist& msg){g_velocity = msg;}

int main(int argc, char** argv)
{
	ros::init(argc, argv, "recorder");
	ros::NodeHandle n;
	
	ros::Subscriber sub_ObjDetect = n.subscribe("/youbotStalker/object_tracking/object_detected", 1000, &callbackObjDetected);
	ros::Subscriber sub_PosX = n.subscribe("/youbotStalker/object_tracking/cam_x_pos", 1000, &callbackPosX);
	//ros::Subscriber sub_PosX = n.subscribe("/chatter2", 1000, &callbackPosX);
	ros::Subscriber sub_PosY = n.subscribe("/youbotStalker/object_tracking/cam_y_pos", 1000, &callbackPosY);
	ros::Subscriber sub_distance = n.subscribe("/youbotStalker/object_tracking/distance", 1000, &callbackDistance);
	ros::Subscriber sub_velocity = n.subscribe("/cmd_vel", 1000, &callbackVelocity);


	ros::Rate r(50); //recording at 40 hz 
	
	char timebuf[50];
	
	time_t rawtime;
	struct tm* timeinfo;
	char filename[80];
	
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(filename, 80, "youbotStalker_%Y-%m-%d_%H-%M.txt", timeinfo);
	
	std::ofstream outFile;
	outFile.open(filename);
	
	if (!outFile.is_open()){
		ROS_ERROR("Cannot create file!");
		return 1;
	}
	
	ros::Time begin = ros::Time::now();
	ros::Duration duration(0.0);
	
	outFile<<"second;userDetected;userPosX;userPosY;userDistance;velLinX;velLinY;velAngZ\n";
	
	while (n.ok()){
		duration = ros::Time::now() - begin;
		sprintf(timebuf, "%.5f", duration.toSec());
		
		if (duration.toSec() - (int)duration.toSec() < 0.02)
			ROS_INFO("File: %s on %.0f seconds", filename, duration.toSec());
		
		outFile<<timebuf<<g_userDetected<<";"<<g_userPosX<<";"<<g_userPosY<<";"<<g_userDistance<<";";
		outFile<<g_velocity.linear.x<<";"<<g_velocity.linear.y<<";"<<g_velocity.angular.z<<"\n";

		ros::spinOnce();
		r.sleep();
	}
	
	ROS_WARN("Recording finished!");
	ROS_INFO("Duration: %.2f seconds", duration.toSec());
	ROS_INFO("Filename: %s", filename);
	outFile.close();
	return 0;
}
