/**
	@file
	aka.leapmotion
	@ingroup	aka

	Masayuki Akamatsu - masayukiakamatsu@mac.com
*/

#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object

#include "Leap.h"

#include <iostream>

#define _USE_MATH_DEFINES // To get definition of M_PI
#include <math.h>

////////////////////////// object struct
typedef struct _akaleapmotion 
{
	t_object	ob;
	int64_t		frame_id_save;
	void		*outlet;
	Leap::Controller	*leap;
} t_akaleapmotion;

///////////////////////// function prototypes
//// standard set
void *akaleapmotion_new(t_symbol *s, long argc, t_atom *argv);
void akaleapmotion_free(t_akaleapmotion *x);
void akaleapmotion_assist(t_akaleapmotion *x, void *b, long m, long a, char *s);

void akaleapmotion_bang(t_akaleapmotion *x);

//////////////////////// global class pointer variable
void *akaleapmotion_class;

//////////////////////// Max functions
int main(void)
{	
	t_class *c;
	
	c = class_new("aka.leapmotion", (method)akaleapmotion_new, (method)akaleapmotion_free, (long)sizeof(t_akaleapmotion), 
				  0L /* leave NULL!! */, A_GIMME, 0);
	
    class_addmethod(c, (method)akaleapmotion_bang, "bang", 0);

	/* you CAN'T call this from the patcher */
    class_addmethod(c, (method)akaleapmotion_assist, "assist", A_CANT, 0);  
	
	class_register(CLASS_BOX, c);
	akaleapmotion_class = c;

	return 0;
}

void akaleapmotion_assist(t_akaleapmotion *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { //inlet
		sprintf(s, "bang to cause the frame data output");
	} 
	else {	// outlet
		sprintf(s, "list(frame data)"); 			
	}
}

void akaleapmotion_free(t_akaleapmotion *x)
{
	delete (Leap::Controller *)(x->leap);
}

void akaleapmotion_bang(t_akaleapmotion *x)
{
	const Leap::Frame frame = x->leap->frame();
	const int64_t frame_id = frame.id();
	
	// ignore the same frame
	if (frame_id == x->frame_id_save) return;
	x->frame_id_save = frame_id;
	
	outlet_anything(x->outlet, gensym("frame_start"), 0, nil);

	const Leap::HandList hands = frame.hands();
	const size_t numHands = hands.count();
	
	t_atom frame_data[3];
	atom_setlong(frame_data, frame_id);
	atom_setlong(frame_data+1, frame.timestamp());
	atom_setlong(frame_data+2, numHands);
	outlet_anything(x->outlet, gensym("frame"), 3, frame_data);
	
	for(size_t i = 0; i < numHands; i++)
	{
		// Hand
		const Leap::Hand &hand = hands[i];
		const int32_t hand_id = hand.id();
		const Leap::FingerList &fingers = hand.fingers();
		const size_t numFingers = fingers.count();
		
		t_atom hand_data[3];
		atom_setlong(hand_data, hand_id);
		atom_setlong(hand_data+1, frame_id);
		atom_setlong(hand_data+2, numFingers);
		outlet_anything(x->outlet, gensym("hand"), 3, hand_data);
		
		for(size_t j = 0; j < numFingers; j++)
		{
			// Finger
			const Leap::Finger &finger = fingers[j];
			const int32_t finger_id = finger.id();
			//const Leap::Ray& tip = finger.tip();
			const Leap::Vector direction = finger.direction();
			const Leap::Vector position = finger.tipPosition();
			const Leap::Vector velocity = finger.tipVelocity();
			const double width = finger.width();
			const double lenght = finger.length();
			const bool isTool = finger.isTool();
			
			t_atom finger_data[15];
			atom_setlong(finger_data, finger_id);
			atom_setlong(finger_data+1, hand_id);
			atom_setlong(finger_data+2, frame_id);
			atom_setfloat(finger_data+3, position.x);
			atom_setfloat(finger_data+4, position.y);
			atom_setfloat(finger_data+5, position.z);
			atom_setfloat(finger_data+6, direction.x);
			atom_setfloat(finger_data+7, direction.y);
			atom_setfloat(finger_data+8, direction.z);
			atom_setfloat(finger_data+9, velocity.x);
			atom_setfloat(finger_data+10, velocity.y);
			atom_setfloat(finger_data+11, velocity.z);
			atom_setfloat(finger_data+12, width);
			atom_setfloat(finger_data+13, lenght);
			atom_setlong(finger_data+14, isTool);
			outlet_anything(x->outlet, gensym("finger"), 15, finger_data);
		}
		
		// Palm
		//const Leap::Ray* palm = hand.palm();
		//if (palm != nil)
		//{
			const Leap::Vector position = hand.palmPosition();
			const Leap::Vector direction = hand.direction();

			t_atom palm_data[14];
			atom_setlong(palm_data, hand_id);
			atom_setlong(palm_data+1, frame_id);
			atom_setfloat(palm_data+2, position.x);
			atom_setfloat(palm_data+3, position.y);
			atom_setfloat(palm_data+4, position.z);
			atom_setfloat(palm_data+5, direction.x);
			atom_setfloat(palm_data+6, direction.y);
			atom_setfloat(palm_data+7, direction.z);
			
			// Palm Velocity
			const Leap::Vector velocity = hand.palmVelocity();
			//if (velocity != nil)
			//{
				atom_setfloat(palm_data+8, velocity.x);
				atom_setfloat(palm_data+9, velocity.y);
				atom_setfloat(palm_data+10, velocity.z);
			//}
			/*
			else
			{
				atom_setfloat(palm_data+8, 0);
				atom_setfloat(palm_data+9, 0);
				atom_setfloat(palm_data+10, 0);
			}
			*/
			// Palm Normal
			const Leap::Vector normal = hand.palmNormal();
			//if (normal != nil)
			//{
				atom_setfloat(palm_data+11, normal.x);
				atom_setfloat(palm_data+12, normal.y);
				atom_setfloat(palm_data+13, normal.z);
			//}
			/*
			else
			{
				atom_setfloat(palm_data+11, 0);
				atom_setfloat(palm_data+12, 0);
				atom_setfloat(palm_data+13, 0);
			}
			*/
			outlet_anything(x->outlet, gensym("palm"), 14, palm_data);
		//}
		
		// Ball
		//const Leap::Ball* ball = hand.ball();
		//if (ball != nil)
		//{
			const Leap::Vector sphereCenter = hand.sphereCenter();
			const double sphereRadius = hand.sphereRadius();
			
			t_atom ball_data[6];
			atom_setlong(ball_data, hand_id);
			atom_setlong(ball_data+1, frame_id);
			atom_setfloat(ball_data+2, sphereCenter.x);
			atom_setfloat(ball_data+3, sphereCenter.y);
			atom_setfloat(ball_data+4, sphereCenter.z);
			atom_setfloat(ball_data+5, sphereRadius);
			outlet_anything(x->outlet, gensym("ball"), 6, ball_data);
		//}
	}
	
	outlet_anything(x->outlet, gensym("frame_end"), 0, nil);
}

void *akaleapmotion_new(t_symbol *s, long argc, t_atom *argv)
{
	t_akaleapmotion *x = NULL;
    
	if (x = (t_akaleapmotion *)object_alloc((t_class *)akaleapmotion_class))
	{
		object_post((t_object *)x, "aka.leapmotion 0.2 for The Leap 0.7.0");

		x->frame_id_save = 0;
		x->outlet = outlet_new(x, NULL);
		
		// Create a controller 
		x->leap = new Leap::Controller;
	}
	return (x);
}
