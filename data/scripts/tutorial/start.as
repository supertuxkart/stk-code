void onStart()
{
    Vec3 created = Vec3(0.0, 0.0, 0.0);
    printVec3(created);
    createTrigger("added_script", created , 30.0);                    	//name,x,y,z,trigger distance
}
