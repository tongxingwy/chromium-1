// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content_public.browser;

import org.chromium.base.annotations.SuppressFBWarnings;

import java.util.ArrayList;

/**
 * A light-weight data structure to encode key information from the accessibility
 * tree for operations that need a quick snapshot of the web content. This is different
 * from BrowserAccessibilityManager.java, which maintains a persistent Android
 * accessibility tree that can be queried synchronously by the Android framework.
 */
@SuppressFBWarnings("URF_UNREAD_PUBLIC_OR_PROTECTED_FIELD")
public class AccessibilitySnapshotNode {

    public int x, y, scrollX, scrollY, width, height;
    public String text;
    public String className;
    public ArrayList<AccessibilitySnapshotNode> children =
            new ArrayList<AccessibilitySnapshotNode>();

    public AccessibilitySnapshotNode(int x, int y, int scrollX, int scrollY, int width,
            int height, String text, String className) {
        this.x = x;
        this.y = y;
        this.scrollX = scrollX;
        this.scrollY = scrollY;
        this.width = width;
        this.height = height;
        this.text = text;
        this.className = className;
    }

    public void addChild(AccessibilitySnapshotNode node) {
        children.add(node);
    }
}
