/**
 * @typedef {Object} Node
 * @property {Number} value
 * @property {Node} left
 * @property {Node} right
 */

/**
 * @param {Number} value - the value to search for
 * @param {Node} node - the node to search for
 * @returns {Node}
 */
function search(value, node)
{
    if (node.value === value)
    {
        return node;
    }
    else if (value< node.value)
    {
        return search(value, node.left);
    } else
    {
        return search(value, node.right);
    }
}


search(1, { value : 2, /**ml:!value,left,right**/

