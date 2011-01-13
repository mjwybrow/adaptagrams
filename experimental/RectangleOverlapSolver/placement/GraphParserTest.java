package placement;

import junit.framework.TestCase;

public class GraphParserTest extends TestCase {
	public void testGraph1(){
		new GraphParser("testdata/biograph010.dot");
	}
}
