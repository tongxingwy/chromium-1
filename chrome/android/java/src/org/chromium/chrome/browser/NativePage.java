// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser;

import android.graphics.Bitmap;
import android.view.View;

/**
 * An interface for pages that will be shown in a tab using Android views instead of html.
 */
public interface NativePage {
    /**
     * @return The View to display the page. This is always non-null.
     */
    View getView();

    /**
     * @return The title of the page.
     */
    String getTitle();

    /**
     * @return The URL of the page.
     */
    String getUrl();

    /**
     * @return The hostname for this page, e.g. "newtab" or "bookmarks".
     */
    public String getHost();

    /**
     * @return The background color of the page.
     */
    int getBackgroundColor();

    /**
     * Updates the native page based on the given url.
     */
    public void updateForUrl(String url);

    /**
     * Called after a page has been removed from the view hierarchy and will no longer be used.
     */
    public void destroy();

    /**
     * @return An unscaled screenshot of the page.
     */
    Bitmap getBitmap();

    /**
     * @return A screenshot of the page scaled to the specified size.
     */
    Bitmap getBitmap(int width, int height);
}
