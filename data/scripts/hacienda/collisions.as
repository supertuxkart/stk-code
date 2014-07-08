void onKartObjectCollision()
{

    displayMessage("Woot! You hit item of ID: " + getCollidingID());
disableAnimation(getCollidingID());
Vec3 explosion_loc = Vec3(79.53,0.07,97.13);
createExplosion(explosion_loc);
}

void onItemObjectCollision()
{

}


void onKartKartCollision()
{

}
