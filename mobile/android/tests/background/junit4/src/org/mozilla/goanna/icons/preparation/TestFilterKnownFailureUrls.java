/* Any copyright is dedicated to the Public Domain.
   http://creativecommons.org/publicdomain/zero/1.0/ */

package org.mozilla.goanna.icons.preparation;

import junit.framework.Assert;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mozilla.goanna.background.testhelpers.TestRunner;
import org.mozilla.goanna.icons.IconDescriptor;
import org.mozilla.goanna.icons.IconRequest;
import org.mozilla.goanna.icons.Icons;
import org.mozilla.goanna.icons.storage.FailureCache;
import org.robolectric.RuntimeEnvironment;

@RunWith(TestRunner.class)
public class TestFilterKnownFailureUrls {
    private static final String TEST_PAGE_URL = "http://www.mozilla.org";
    private static final String TEST_ICON_URL = "https://example.org/favicon.ico";

    @Before
    public void setUp() {
        // Make sure we always start with an empty cache.
        FailureCache.get().evictAll();
    }

    @Test
    public void testFilterDoesNothingByDefault() {
        final IconRequest request = Icons.with(RuntimeEnvironment.application)
                .pageUrl(TEST_PAGE_URL)
                .icon(IconDescriptor.createGenericIcon(TEST_ICON_URL))
                .build();

        Assert.assertEquals(1, request.getIconCount());

        final Preparer preparer = new FilterKnownFailureUrls();
        preparer.prepare(request);

        Assert.assertEquals(1, request.getIconCount());
    }

    @Test
    public void testFilterKnownFailureUrls() {
        final IconRequest request = Icons.with(RuntimeEnvironment.application)
                .pageUrl(TEST_PAGE_URL)
                .icon(IconDescriptor.createGenericIcon(TEST_ICON_URL))
                .build();

        Assert.assertEquals(1, request.getIconCount());

        FailureCache.get().rememberFailure(TEST_ICON_URL);

        final Preparer preparer = new FilterKnownFailureUrls();
        preparer.prepare(request);

        Assert.assertEquals(0, request.getIconCount());
    }
}
