/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



package org.openoffice.accessibility.awb.tree;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.lang.DisposedException;
import com.sun.star.lang.IndexOutOfBoundsException;

import org.openoffice.accessibility.misc.OfficeConnection;
import org.openoffice.accessibility.awb.event.EventQueue;

import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreeNode;


public class AccessibilityTreeModel
    extends DefaultTreeModel
{
    public AccessibilityTreeModel ()
    {
        super (null);
		setAsksAllowsChildren (false);

        SetRootNode();
    }



    /** Release all resources.
    */
    synchronized public void Dispose ()
    {
		Clear ();
    }



    /** Calls to this method are dispatched to the given node but are
        observed for exceptions.  
    */
    synchronized public boolean isLeaf (Object aObject)
    {
        boolean bIsLeaf = true;

        if (aObject != null)
        {
            AccessibilityNode aNode = (AccessibilityNode)aObject;
            try
            {
                bIsLeaf = aNode.isLeaf();
            }
            catch (DisposedException aException)
            {
                System.out.println ("node is disposed. removing it");
                /*                TreeNode aParent = aNode.GetParent();
                int nIndexInParent = aParent.getIndex (aNode);
                aNode.removeFromParent ();
                System.out.println ("" + aParent + " # " + aNode + " # "+ nIndexInParent);
                nodesWereRemoved (
                    aParent, new int[]{nIndexInParent}, new
                    Object[]{aNode});
                */
            }
            catch (Exception aException)
            {
                System.err.println ("caught exception in AccessibilityTreeModel.isLeaf():" 
                    + aException);
                aException.printStackTrace (System.err);
            }
        }

        return bIsLeaf;
    }




    synchronized public int getChildCount (Object aObject)
    {
        AccessibilityNode aNode = (AccessibilityNode)aObject;
        return aNode.getChildCount();
    }




    /** Return the requested child of aParent.  If that child is not yet
        known to the parent then try to create it.
    */
    synchronized public Object getChild (Object aParent, final int nIndex)
    {
        AccessibilityNode aChild = null;
        
        final AccessibilityNode aParentNode = (AccessibilityNode)aParent;

        // Try to get an existing child from the super class object.
        aChild = aParentNode.GetChildNoCreate (nIndex);

        // When the requested child does not yet exist and this node is not a
        // special node then create a new node.
        if (aChild == null)
        {
            aChild = aParentNode.CreateChild (nIndex);
            aParentNode.SetChild ((AccessibilityNode)aChild, nIndex);
            /*            EventQueue.Instance().AddEvent (new Runnable() { public void run() {
                AccessibilityTreeModel.this.nodeWasInserted (
                    aParentNode, nIndex);
            }});
            */        }

        return aChild;
    }


    synchronized public void nodeWasInserted (AccessibilityNode aParent, int nIndex)
    {
        nodesWereInserted (aParent, new int[]{nIndex});
        nodeStructureChanged (aParent);

    }




    /** Clear the tree so that afterwards it has only the root node.
    */
    public void Clear ()
    {
        AccessibilityNode aRoot = (AccessibilityNode)getRoot();
        aRoot.RemoveAllChildren();
        SetRootNode();
		nodeStructureChanged (aRoot);
    }




    private void SetRootNode ()
    {
        OfficeConnection aConnection = OfficeConnection.Instance();
        AccessibilityNode aRoot;
        if (aConnection!=null && aConnection.IsValid())
            aRoot = new AccessibilityNode ("<connected>");
        else
            aRoot = new AccessibilityNode ("<not connected>");
        setRoot (aRoot);
    }


	

	/** Add a new child to the root node.
    */
	public synchronized void AddTopLevelNode (AccessibilityNode aNode)
    {
        if (aNode != null)
        {
            if ( ! OfficeConnection.Instance().IsValid())
            {
                setRoot (null);
            }

            AccessibilityNode aRoot = (AccessibilityNode)getRoot();
            if (aRoot == null)
            {
                aRoot = new AccessibilityNode ("<connected>");
                setRoot (aRoot);
            }

            aNode.SetParent (aRoot);
            aRoot.Append (aNode);
			nodesWereInserted (aRoot, new int[]{aRoot.getIndex (aNode)});
        }
    }




	/** Remove a node that is a direct child of the root.
	*/
    public synchronized void RemoveTopLevelNode (AccessibilityNode aNode)
    {
		AccessibilityNode aRoot = (AccessibilityNode)getRoot();
		if (aRoot != null)
		{
			int nIndex = aRoot.getIndex (aNode);
			aRoot.Remove (aNode);
			nodesWereRemoved (aRoot, new int[]{nIndex}, new Object[]{aNode});
		}
    }
}
