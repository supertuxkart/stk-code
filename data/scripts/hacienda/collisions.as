void onKartObjectCollision()
{
    if (getCollidingID()=="stklib_fitchBarrelTestDestroyed_a"){
int kart_id = getCollidingKart1();
Vec3 location = getKartLocation(kart_id);
createExplosion(location);
disableAnimation("stklib_fitchBarreltest_a_main");
}
else {
    displayMessage("Woot! You hit item of ID: " + getCollidingID());
disableAnimation(getCollidingID());
Vec3 explosion_loc = Vec3(79.53,0.07,97.13);
createExplosion(explosion_loc);
}
}

void onItemObjectCollision()
{

}


void onKartKartCollision()
{

}
