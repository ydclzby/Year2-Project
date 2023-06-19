using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Player : MonoBehaviour
{
    public GameObject circlePrefab;


    public struct Node
    { // length with 2
        public Vector2 position; // x, y
        public bool[,] open; // North[haveOpen,visited], ... , West[haveOpen,visited]
        public int entrance; // 0 north, 1 east, 2 south, 3 west
    }
    private List<Node> nodes;

    public float speed;
    private float FRONTTHRESHHOLD = 1f;
    private float RIGHTTHRESHHOLD = 1f;
    private float SCANTOLORANCE = 2f;
    private float NODEPOSITIONTOLORANCE = 1.3f;
    public float distanceToLeft;
    public float distanceToRight;
    public float distanceToFront;
    public float distanceToBack;
    private const int NORTH = 0;
    private const int EAST = 1;
    private const int SOUTH = 2;
    private const int WEST = 3;


    // Start is called before the first frame update
    void Start()
    {
        wallLayer = LayerMask.GetMask("wallLayer");
        nodes = new List<Node>();
    }

    // Update is called once per frame

    public LayerMask wallLayer;
    private bool haveRightTurned = false;
    //public bool haveLeftTurned = false;
    public Vector3 objectFaceDirection;
    public int faceDir = 0;
    public bool haveTurnedSinceJunction = false;
    public bool onNode = false;
    public bool onNodeFirst = true;
    bool[] avaliableRoute = new bool[4]; // front, left, right, back
    void Update()
    {
        faceDir = currentFacing();
        // Debug.Log("Currently facing: " + convertIntToDir(faceDir));
        updateWallDist();
        moveForward();
        scanWTolorance(); // it will change avaliableRoute
        onNode = isOnNode();
        if (isOnNode())
        {
            if (onNodeFirst)
            {
                transform.Translate(0, 0.2f, 0);
                // Debug.Log("I'm on node");

                Node nodeTemp = new Node();

                // translate north by 1 unit

                if (doNodeExist())
                { // if node exist
                    Debug.Log("Updating Node");
                    Node tempNode = retrieveNode();
                    updateOpen(tempNode); // mark the position of entrance as visited
                    onNodeFirst = false;
                }
                else
                { // if node not exist
                    Debug.Log("Adding Node");
                    // Debug.Log("Currently facing: " + convertIntToDir(faceDir));
                    onNodeFirst = false;
                    addNode();
                    spawnCircle();
                }
                // // turn to the unvisited direction
                nodeTemp = retrieveNode();
                if(allNodeAllOpenVisited()){
                    Debug.Log("All node all open visited");
                    // disable the current script
                    this.enabled = false;
                }
                determineDirectionAndTurn(nodeTemp);
                printNode(nodeTemp);
                // // Debug.Log("I'm on node");
            }
            // Debug.Log("I'm on node");
        }
        else
        {
            onNodeFirst = true;
            if (scanFront())
            {
                //haveLeftTurned = true;
                turnLeft90();
                // scanAvoidDoubleCount = true;

            }
            else if (!scanRight() && !haveRightTurned)
            {
                // Debug.Log("isOnNode: " + isOnNode());
                // Debug.Log("onNodeFirst: " + onNode);
                // Debug.Log("I'm turning right");
                haveRightTurned = true;
                // because Unity don't have delay. It can be replaced by moveforward and delay
                // transform.Translate(0, 0.1f, 0);
                turnRight90();
                // scanAvoidDoubleCount = true;
            }
            else if (scanRight())
            {
                haveRightTurned = false;
            }
        }


    }
    void printNode(Node node)
    {
        // Debug.Log("Node position " + node.position.x + " " + node.position.y);
        bool[,] open = node.open;
        Debug.Log("Node open: " + open[0, 0] + " " + open[1, 0] + " " + open[2, 0] + " " + open[3, 0]);
        // Debug.Log("North: " + open[0, 0] + " Visited: " + open[0, 1]);
        // Debug.Log("East: " + open[1, 0] + " Visited: " + open[1, 1]);
        // Debug.Log("South: " + open[2, 0] + " Visited: " + open[2, 1]);
        // Debug.Log("West: " + open[3, 0] + " Visited: " + open[3, 1]);
        if (node.entrance == NORTH)
        {
            Debug.Log("Entrance: North");
        }
        else if (node.entrance == EAST)
        {
            Debug.Log("Entrance: East");
        }
        else if (node.entrance == SOUTH)
        {
            Debug.Log("Entrance: South");
        }
        else if (node.entrance == WEST)
        {
            Debug.Log("Entrance: West");
        }
    }

    void determineDirectionAndTurn(Node node)
    {
        bool[,] open = node.open;
        int currentlyFacing = faceDir;
        for (int i = 6; i >= 2; i--)
        {
            if (open[i % 4, 0])
            { // if this direction have open
                if (!open[i % 4, 1])
                { // if this direction have open but not visited
                    if (i % 4 == (currentlyFacing + 1) % 4)
                    {
                        Debug.Log("Force turning right");
                        turnRight90();
                        open[i % 4, 1] = true;
                        return;
                    }
                    if (i % 4 == currentlyFacing)
                    {
                        Debug.Log("Force moving forward");
                        moveForward();
                        open[i % 4, 1] = true;
                        return;
                    }
                    if (i % 4 == (currentlyFacing - 1) % 4)
                    {
                        Debug.Log("Force turning left");
                        turnLeft90();
                        open[i % 4, 1] = true;
                        return;
                    }
                }
            }
        }
        // if all direction have been visited
        // turn to node.entrance
        if (node.entrance == NORTH)
        {
            Debug.Log("recur north");
            turnNorth();
        }
        else if (node.entrance == EAST)
        {
            Debug.Log("recur east");
            turnEast();
        }
        else if (node.entrance == SOUTH)
        {
            Debug.Log("recur south");
            turnSouth();
        }
        else if (node.entrance == WEST)
        {
            Debug.Log("recur west");
            turnWest();
        }
    }

    void addNode()
    {
        Node nodeTemp = new Node();
        bool[,] openTemp = computeOpen();
        Vector2 positionTemp = transform.position;
        nodeTemp.position = positionTemp;
        nodeTemp.open = openTemp;

        nodeTemp.entrance = (faceDir + 2) % 4;
        nodes.Add(nodeTemp);
    }

    string convertIntToDir(int i)
    {
        if (i == NORTH)
        {
            return "North";
        }
        else if (i == EAST)
        {
            return "East";
        }
        else if (i == SOUTH)
        {
            return "South";
        }
        else if (i == WEST)
        {
            return "West";
        }
        return "Error";
    }

    bool isSameNode()
    {
        foreach (Node node in nodes)
        {
            if (Mathf.Abs(node.position.x - transform.position.x) < NODEPOSITIONTOLORANCE && Mathf.Abs(node.position.y - transform.position.y) < NODEPOSITIONTOLORANCE)
            {
                return true;
            }
        }
        return false;
    }

    bool allNodeAllOpenVisited()
    {
        foreach (Node node in nodes)
        {
            bool[,] open = node.open;
            for (int i = 0; i < open.GetLength(0); i++)
            { // for each direction
                if (open[i, 0])
                { // if this direction have open
                    if (!open[i, 1])
                    { // if this direction have open but not visited
                        return false;
                    }
                }
            }
        }
        return true;
    }
    bool isOnNode()
    {
        return threeOrMoreSatisfied(avaliableRoute);
    }


    Node retrieveNode()
    {
        Vector2 position = transform.position;
        foreach (Node node in nodes)
        {
            if (Mathf.Abs(node.position.x - position.x) < NODEPOSITIONTOLORANCE && Mathf.Abs(node.position.y - position.y) < NODEPOSITIONTOLORANCE)
            {
                return node;
            }
        }
        return new Node();
    }
    bool allVisited(bool[,] open)
    {
        bool flag = true;
        for (int i = 0; i < open.GetLength(0); i++)
        {
            if (open[i, 0])
            { // i.e this place have open
                flag = flag && open[i, 1]; // i.e this place have open and visited
            }
        }
        return flag;
    }

    int determineOpenAndUnvisited(bool[,] open)
    {
        for (int i = 0; i < open.GetLength(0); i++)
        {
            if (open[i, 0])
            { // i.e this place have open
                if (!open[i, 1])
                { // i.e this place have open and unvisited
                    open[i, 1] = true; // set it to visited
                    return i;
                }
            }
        }
        return -1;
    }

    void updateOpen(Node node)
    {
        bool[,] openTemp = node.open;

        openTemp[(faceDir + 2) % 4, 1] = true; // mark the opposite of current facing as visited
    }
    bool doNodeExist()
    {
        Vector2 position = transform.position;
        foreach (Node node in nodes)
        {
            // Debug.Log(Mathf.Abs(node.position.x - position.x) + " " + Mathf.Abs(node.position.y - position.y));
            // Debug.Log(Mathf.Abs(node.position.x - position.x) < NODEPOSITIONTOLORANCE);
            // Debug.Log(Mathf.Abs(node.position.y - position.y) < NODEPOSITIONTOLORANCE);
            if (Mathf.Abs(node.position.x - position.x) < NODEPOSITIONTOLORANCE && Mathf.Abs(node.position.y - position.y) < NODEPOSITIONTOLORANCE)
            {
                return true;
            }
        }
        return false;
    }

    bool[,] retrieveOpen(Vector2 position)
    {
        foreach (Node node in nodes)
        {
            if (Mathf.Abs(node.position.x - position.x) < NODEPOSITIONTOLORANCE && Mathf.Abs(node.position.y - position.y) < NODEPOSITIONTOLORANCE)
            {
                return node.open;
            }
        }
        return null;
    }

    void spawnCircle()
    {
        Vector2 position = transform.position;
        GameObject circle = Instantiate(circlePrefab, position, Quaternion.identity);
        SpriteRenderer spriteRenderer = circle.GetComponent<SpriteRenderer>();
        spriteRenderer.color = Color.black;
    }

    // 0 degree is north, 90 degree is west, 180 degree is south, 270 degree is east
    bool[,] computeOpen()
    {
        bool[,] open = new bool[4, 2];
        int currentlyFacing = faceDir;
        Debug.Log("Currently facing: " + convertIntToDir(faceDir));
        open[currentlyFacing, 0] = distanceToFront > SCANTOLORANCE;
        open[(currentlyFacing + 1) % 4, 0] = distanceToRight > SCANTOLORANCE;
        open[(currentlyFacing + 2) % 4, 0] = true; // where rover come from
        open[(currentlyFacing + 2) % 4, 1] = true;
        open[(currentlyFacing + 3) % 4, 0] = distanceToLeft > SCANTOLORANCE;

        return open;
    }

    void scanWTolorance()
    {
        avaliableRoute[0] = distanceToFront > SCANTOLORANCE;
        avaliableRoute[1] = distanceToLeft > SCANTOLORANCE;
        avaliableRoute[2] = distanceToRight > SCANTOLORANCE;
        avaliableRoute[3] = distanceToBack > SCANTOLORANCE;
    }

    bool twoOrMoreSatisfied(bool[] open)
    {
        int count = 0;
        for (int i = 0; i < open.Length; i++)
        {
            if (open[i])
            {
                count++;
            }
        }
        if (count >= 2)
        {
            return true;
        }
        return false;
    }

    bool threeOrMoreSatisfied(bool[] open)
    {
        int count = 0;
        for (int i = 0; i < open.Length; i++)
        {
            if (open[i])
            {
                count++;
            }
        }
        if (count >= 3)
        {
            return true;
        }
        return false;
    }

    void turnNorth()
    {
        Quaternion currentRotation = transform.rotation;

        // Calculate the desired rotation
        Quaternion desiredRotation = Quaternion.Euler(0f, 0f, 0f); // Example: Rotate by 45 degrees around the Z-axis
        transform.rotation = desiredRotation;
    }
    void turnWest()
    {
        Quaternion currentRotation = transform.rotation;

        // Calculate the desired rotation
        Quaternion desiredRotation = Quaternion.Euler(0f, 0f, 90f); // Example: Rotate by 45 degrees around the Z-axis
        transform.rotation = desiredRotation;
    }
    void turnSouth()
    {
        Quaternion currentRotation = transform.rotation;

        // Calculate the desired rotation
        Quaternion desiredRotation = Quaternion.Euler(0f, 0f, 180f); // Example: Rotate by 45 degrees around the Z-axis
        transform.rotation = desiredRotation;
    }
    void turnEast()
    {
        Quaternion currentRotation = transform.rotation;

        // Calculate the desired rotation
        Quaternion desiredRotation = Quaternion.Euler(0f, 0f, 270f); // Example: Rotate by 45 degrees around the Z-axis
        transform.rotation = desiredRotation;
    }
    void turnAround()
    {
        transform.Rotate(0, 0, 180);
    }
    bool scanRight()
    {
        return distanceToRight < RIGHTTHRESHHOLD;
    }
    bool scanLeft()
    {
        return distanceToLeft < RIGHTTHRESHHOLD;
    }
    bool scanFront()
    {
        return distanceToFront < FRONTTHRESHHOLD;
    }
    bool scanBack()
    {
        return distanceToBack < FRONTTHRESHHOLD;
    }

    int currentFacing()
    {
        if (transform.eulerAngles.z == 0)
        { // north
            return NORTH;
        }
        else if (transform.eulerAngles.z == 90)
        { //west
            return WEST;
        }
        else if (transform.eulerAngles.z == 180)
        { // south
            return SOUTH;
        }
        else if (transform.eulerAngles.z == 270)
        { // east
            return EAST;
        }
        return -1;
    }
    void moveForward()
    {
        transform.Translate(0, speed * Time.deltaTime, 0);
    }
    void turnRight90()
    {
        transform.Rotate(0, 0, -90);
    }
    void turnLeft90()
    {
        transform.Rotate(0, 0, 90);
    }

    void updateWallDist()
    {
        // Calculate raycast directions based on player's rotation
        Vector2 leftDirection = -transform.right;
        Vector2 rightDirection = transform.right;
        Vector2 forwardDirection = transform.up;
        Vector2 backDirection = -transform.up;
        // Raycast to the left
        RaycastHit2D hitLeft = Physics2D.Raycast(transform.position, leftDirection, Mathf.Infinity, wallLayer);
        if (hitLeft.collider != null && !hitLeft.collider.CompareTag("Player"))
        {
            distanceToLeft = hitLeft.distance;
        }
        else
        {
            // No wall detected in this direction
            distanceToLeft = Mathf.Infinity;
        }

        // Raycast to the right
        RaycastHit2D hitRight = Physics2D.Raycast(transform.position, rightDirection, Mathf.Infinity, wallLayer);
        if (hitRight.collider != null && !hitRight.collider.CompareTag("Player"))
        {
            distanceToRight = hitRight.distance;
        }
        else
        {
            // No wall detected in this direction
            distanceToRight = Mathf.Infinity;
        }

        // Raycast forward
        RaycastHit2D hitFront = Physics2D.Raycast(transform.position, forwardDirection, Mathf.Infinity, wallLayer);
        if (hitFront.collider != null && !hitFront.collider.CompareTag("Player"))
        {
            distanceToFront = hitFront.distance;
        }
        else
        {
            // No wall detected in this direction
            distanceToFront = Mathf.Infinity;
        }

        // Raycast back
        RaycastHit2D hitBack = Physics2D.Raycast(transform.position, backDirection, Mathf.Infinity, wallLayer);
        if (hitBack.collider != null && !hitBack.collider.CompareTag("Player"))
        {
            distanceToBack = hitBack.distance;
        }
        else
        {
            // No wall detected in this direction
            distanceToBack = Mathf.Infinity;
        }
    }
    private void OnCollisionEnter2D(Collision2D collision)
    {
        if (collision.gameObject.tag == "Walls")
        {
            // Debug.Log("Hit Wall");
        }
    }
    private void manuelMoveAndRotate()
    {
        if (Input.GetKeyUp("q"))
        {
            transform.Rotate(0, 0, 90);
        }
        if (Input.GetKeyUp("e"))
        {
            transform.Rotate(0, 0, -90);
        }

        if (Input.GetKey("w"))
        {
            transform.Translate(0, speed * Time.deltaTime, 0);
        }
        if (Input.GetKey("s"))
        {
            transform.Translate(0, -speed * Time.deltaTime, 0);
        }
        if (Input.GetKey("a"))
        {
            transform.Translate(-speed * Time.deltaTime, 0, 0);
        }
        if (Input.GetKey("d"))
        {
            transform.Translate(speed * Time.deltaTime, 0, 0);
        }
    }
}
