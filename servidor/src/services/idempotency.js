/** Órdenes ya dispensadas (POC — memoria RAM; se pierde al reiniciar Railway). */
const processedOrderIds = new Set();

function wasDispensed(orderId) {
  return processedOrderIds.has(orderId);
}

function markDispensed(orderId) {
  processedOrderIds.add(orderId);
}

function clearForTests() {
  processedOrderIds.clear();
}

module.exports = { wasDispensed, markDispensed, clearForTests };
