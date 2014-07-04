void onCollision()
{

    displayMessage("Woot! You hit item of ID: " + getCollidingID());
disableAnimation(getCollidingID());
}
