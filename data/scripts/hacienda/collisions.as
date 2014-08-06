void onKartObjectCollision()
{
    if (getCollidingID()=="stklib_fitchBarreltest_a_main"){
int kart_id = getCollidingKart1();
Vec3 location = getKartLocation(kart_id);
createExplosion(location);
disable("stklib_fitchBarreltest_a_main");
enable("stklib_fitchBarrelTestDestroyed_a");
enable("stklib_fitchBarrelTestcovert_a");
}
else {
   /* displayMessage("Woot! You hit item of ID: " + getCollidingID());
disable(getCollidingID());
Vec3 explosion_loc = Vec3(79.53,0.07,97.13);
createExplosion(explosion_loc);*/
}
}

void onItemObjectCollision()
{

}


void onKartKartCollision()
{

}
