package placement;

import java.awt.Rectangle;
import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.util.ArrayList;
import java.util.Hashtable;

class Edge {
	Node start;
	Node end;
	Edge(Node start, Node end) {
		this.start = start;
		this.end = end;
	}
}
class Node {
	Rectangle rectangle;
	String id;
	ArrayList<Edge> edges;
	Node(String id,int x,int y,int w,int h) {
		rectangle = new Rectangle(x-w/2,y-w/2,w,h);
		this.id = id;
		edges = new ArrayList<Edge>();
	}
	void addEdge(Edge e) {
		edges.add(e);
	}
	public Point2D getPos() {
		Point2D p = rectangle.getLocation();
		double x = rectangle.width/2.0;
		double y = rectangle.height/2.0;
		return new Point2D.Double(p.getX()+x,p.getY()+y);
	}
}
public class Graph {
	Hashtable<String,Node> nodes = new Hashtable<String,Node>();
	ArrayList<Edge> edges = new ArrayList<Edge>();
	public ArrayList<Rectangle2D> getRectangles() {
		ArrayList<Rectangle2D> rectangles = new ArrayList<Rectangle2D>();
		for(Node n:nodes.values()) {
			rectangles.add(n.rectangle);
		}
		return rectangles;
	}
	public ArrayList<Line2D> getLines() {
		ArrayList<Line2D> lines = new ArrayList<Line2D>();
		for(Edge e:edges) {
			lines.add(new Line2D.Double(e.start.getPos(),e.end.getPos()));
		}
		return lines;
	}
	public void addNode(String id,int x,int y,int w, int h) {
		nodes.put(id,new Node(id,x,y,w,h));
	}
	public void addEdge(String id1, String id2) {
		edges.add(new Edge(nodes.get(id1),nodes.get(id2)));
	}
}
