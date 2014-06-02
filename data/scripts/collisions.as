void onCollision()
{
/*
Currently activates on Kart collisions
*/
    displayMessage("Whoa! Road rage... between Kart: " + getCollidingKart1() + " - " + getCollidingKart2() + " " + getCollisionType() + "ID is : " + getCollidingID());
    disableAnimation(getCollidingID());
}
