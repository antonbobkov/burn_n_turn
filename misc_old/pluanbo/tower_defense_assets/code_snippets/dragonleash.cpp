struct Polar
{
	float r,a;
	Polar():r(0),a(0){}
	Polar(float a_, float r_):a(a_),r(r_){}
	Polar(fPoint p):r(p.length())
	{
		if(p.y == 0 && p.x == 0)
			a = 0;
		else
			a = atan2(p.y,p.x);
	}
	Polar operator*(Polar p) { return Polar(a+p.a, r*p.r); }
	
	fPoint TofPoint(){ return fPoint(r*cos(a),r*sin(b)); }
};

struct DragonLeash
{
	fPoint lastVel;
	//tilt in radians per frame
	float tilt;
	float speed; //in pixels/frame
	float trackballScaleFactor;
	float naturalScaleFactor;
	DragonLeash():lastVel(0,-1)
	{
		tilt = 0;
		speed = 10;
		scaleFactor = 1/300;
		naturalScaleFactor = .1;
	}

	void ModifyTilt(Point trackball)
	{
		tilt += tilt*naturalScaleFactor;
		tilt += trackball.x*trackballScaleFactor;
	}

	fPoint GetNewVelocity(Point trackball)
	{
		ModifyTilt(trackball);
		p = lastVel.toPolar();
		p.r 
		p.a += tilt;
		lastVel = p.tofPoint();
		return lastVel;
	}
};